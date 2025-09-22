#include "Processor.h"
#include "../Lexer/Lexer.h"
#include "../Parser/Parser.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <map>
#include <set>
#include <iostream>
#include <list>

namespace YINI
{
    // Forward declarations
    void expandValue(std::unique_ptr<Value>& value, const std::map<std::string, const Value*>& macros, std::set<std::string>& expansion_stack);
    std::unique_ptr<Value> cloneValue(const Value& original);

    Processor::Processor(std::unique_ptr<YiniFile> yini_file)
        : m_yini_file(std::move(yini_file))
    {
    }

    std::unique_ptr<YiniFile> Processor::process()
    {
        processIncludes();
        processInheritance();
        processMacros();
        return std::move(m_yini_file);
    }

    std::unique_ptr<Value> cloneValue(const Value& original)
    {
        auto new_value = std::make_unique<Value>();
        std::visit([&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, Array>)
            {
                auto new_array = Array();
                for (const auto& item : arg)
                {
                    new_array.push_back(cloneValue(*item));
                }
                new_value->data = std::move(new_array);
            }
            else if constexpr (std::is_same_v<T, Map>)
            {
                auto new_map = Map();
                for (const auto& [key, val] : arg)
                {
                    new_map[key] = cloneValue(*val);
                }
                new_value->data = std::move(new_map);
            }
            else
            {
                new_value->data = arg;
            }
        }, original.data);
        return new_value;
    }

    void Processor::processIncludes()
    {
        auto include_it = m_yini_file->sections.find("#include");
        if (include_it == m_yini_file->sections.end())
        {
            return;
        }

        // 1. Extract filenames to avoid iterator invalidation and copy issues
        std::vector<std::string> filenames;
        for (const auto& kvp : include_it->second.pairs)
        {
            if (!kvp.is_quick_registration || !std::holds_alternative<String>(kvp.value->data))
            {
                throw std::runtime_error("Invalid entry in [#include] section. Must be of the form '+= \"filename.yini\"'");
            }
            filenames.push_back(std::get<String>(kvp.value->data));
        }

        // 2. Erase the [#include] section now
        m_yini_file->sections.erase("#include");

        // 3. Process the files
        for (const auto& filename : filenames)
        {
            std::ifstream file_stream(filename);
            if (!file_stream)
            {
                throw std::runtime_error("Failed to open include file: " + filename);
            }

            std::stringstream buffer;
            buffer << file_stream.rdbuf();
            std::string file_content = buffer.str();

            // Recursively process the included file
            Lexer lexer(file_content);
            Parser parser(lexer);
            auto included_ast = parser.parse();
            Processor include_processor(std::move(included_ast));
            auto processed_included_ast = include_processor.process();

            // Merge the processed sections
            for (auto& [name, included_section] : processed_included_ast->sections)
            {
                if (name == "#define" || name == "#include") continue;

                auto main_it = m_yini_file->sections.find(name);
                if (main_it == m_yini_file->sections.end())
                {
                    // Section does not exist in main file, so just move it over.
                    m_yini_file->sections[name] = std::move(included_section);
                }
                else
                {
                    // Section exists, merge key-value pairs. Main file's keys override included file's.
                    auto& main_section = main_it->second;
                    std::map<std::string, KeyValuePair> merged_pairs_map;

                    // Add included section's pairs first
                    for (const auto& kvp : included_section.pairs)
                    {
                        merged_pairs_map.insert_or_assign(kvp.key, KeyValuePair(kvp.key, cloneValue(*kvp.value), kvp.is_quick_registration));
                    }

                    // Add main section's pairs, overriding included ones
                    for (const auto& kvp : main_section.pairs)
                    {
                        merged_pairs_map.insert_or_assign(kvp.key, KeyValuePair(kvp.key, cloneValue(*kvp.value), kvp.is_quick_registration));
                    }

                    main_section.pairs.clear();
                    for (auto& [key, kvp] : merged_pairs_map)
                    {
                        main_section.pairs.push_back(std::move(kvp));
                    }
                }
            }
        }
    }

    void Processor::processInheritance()
    {
        // ... (rest of the code is the same)
        std::map<std::string, std::vector<std::string>> adj;
        std::map<std::string, int> in_degree;

        for (const auto& [name, section] : m_yini_file->sections)
        {
            in_degree[name] = 0;
        }

        for (const auto& [name, section] : m_yini_file->sections)
        {
            for (const auto& parent_name : section.inherits)
            {
                adj[parent_name].push_back(name);
                in_degree[name]++;
            }
        }

        std::list<std::string> queue;
        for (const auto& [name, degree] : in_degree)
        {
            if (degree == 0)
            {
                queue.push_back(name);
            }
        }

        std::vector<std::string> sorted_order;
        while (!queue.empty())
        {
            std::string u = queue.front();
            queue.pop_front();
            sorted_order.push_back(u);

            for (const auto& v : adj[u])
            {
                in_degree[v]--;
                if (in_degree[v] == 0)
                {
                    queue.push_back(v);
                }
            }
        }

        if (sorted_order.size() != m_yini_file->sections.size())
        {
            throw std::runtime_error("Circular dependency detected in section inheritance.");
        }

        for (const auto& section_name : sorted_order)
        {
            auto& section = m_yini_file->sections.at(section_name);
            if (section.inherits.empty()) continue;

            std::map<std::string, KeyValuePair> merged_pairs_map;

            for (const auto& parent_name : section.inherits)
            {
                if (m_yini_file->sections.find(parent_name) == m_yini_file->sections.end())
                {
                    throw std::runtime_error("Inherited section '" + parent_name + "' not found.");
                }
                const auto& parent_section = m_yini_file->sections.at(parent_name);
                for (const auto& kvp : parent_section.pairs)
                {
                    merged_pairs_map.insert_or_assign(kvp.key, KeyValuePair(kvp.key, cloneValue(*kvp.value), kvp.is_quick_registration));
                }
            }

            for (const auto& kvp : section.pairs)
            {
                 merged_pairs_map.insert_or_assign(kvp.key, KeyValuePair(kvp.key, cloneValue(*kvp.value), kvp.is_quick_registration));
            }

            section.pairs.clear();
            for (auto& [key, kvp] : merged_pairs_map)
            {
                section.pairs.push_back(std::move(kvp));
            }
            section.inherits.clear();
        }
    }

    void expandValue(std::unique_ptr<Value>& value, const std::map<std::string, const Value*>& macros, std::set<std::string>& expansion_stack)
    {
        if (!value) return;

        if (std::holds_alternative<Macro>(value->data))
        {
            std::string macro_name = std::get<Macro>(value->data).name;
            if (expansion_stack.count(macro_name))
            {
                throw std::runtime_error("Circular macro reference detected for: " + macro_name);
            }
            auto it = macros.find(macro_name);
            if (it == macros.end())
            {
                throw std::runtime_error("Undefined macro: " + macro_name);
            }
            value = cloneValue(*it->second);
            expansion_stack.insert(macro_name);
            expandValue(value, macros, expansion_stack);
            expansion_stack.erase(macro_name);
        }
        else if (std::holds_alternative<Array>(value->data))
        {
            auto& array = std::get<Array>(value->data);
            for (auto& element : array)
            {
                expandValue(element, macros, expansion_stack);
            }
        }
        else if (std::holds_alternative<Map>(value->data))
        {
            auto& map = std::get<Map>(value->data);
            for (auto& [key, val] : map)
            {
                expandValue(val, macros, expansion_stack);
            }
        }
    }

    void Processor::processMacros()
    {
        std::map<std::string, const Value*> macros;
        auto define_it = m_yini_file->sections.find("#define");
        if (define_it != m_yini_file->sections.end())
        {
            for (const auto& kvp : define_it->second.pairs)
            {
                macros[kvp.key] = kvp.value.get();
            }
        }

        for (auto& [section_name, section] : m_yini_file->sections)
        {
            if (section.is_define_section) continue;
            for (auto& kvp : section.pairs)
            {
                std::set<std::string> expansion_stack;
                expandValue(kvp.value, macros, expansion_stack);
            }
        }
    }
}
