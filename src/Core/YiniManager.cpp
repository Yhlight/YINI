#include "YiniManager.h"
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <regex>
#include <cstdio>
#include <variant>

namespace YINI
{
    YiniManager::YiniManager() = default;

    void YiniManager::load(const std::string& filepath)
    {
        m_filepath = filepath;
        m_schema = nullptr; // Reset schema on new load
        std::set<std::string> loaded_files;
        auto final_ast = load_file(filepath, loaded_files);

        // Find and extract the schema node from the AST
        for (auto it = final_ast.begin(); it != final_ast.end(); ) {
            if (auto* schema_node = dynamic_cast<Schema*>(it->get())) {
                if (m_schema) {
                    // In a more advanced implementation, we might merge schemas.
                    // For now, we'll just take the first one found.
                } else {
                    m_schema.reset(static_cast<Schema*>(it->release()));
                }
                it = final_ast.erase(it);
            } else {
                ++it;
            }
        }

        m_interpreter.interpret(final_ast);
    }

    const Interpreter& YiniManager::get_interpreter() const
    {
        return m_interpreter;
    }

    YiniValue YiniManager::get_value(const std::string& section, const std::string& key)
    {
        if (m_interpreter.resolved_sections.count(section) && m_interpreter.resolved_sections[section].count(key)) {
            YiniValue& value = m_interpreter.resolved_sections[section][key];
            if (auto* dyna_val_ptr = std::get_if<std::unique_ptr<DynaValue>>(&value.m_value)) {
                return (*dyna_val_ptr)->get();
            }
            return value;
        }
        throw std::runtime_error("Value not found for section '" + section + "' and key '" + key + "'.");
    }

    void YiniManager::set_value(const std::string& section, const std::string& key, YiniValue new_value)
    {
        if (m_interpreter.resolved_sections.count(section) && m_interpreter.resolved_sections[section].count(key)) {
            YiniValue& value = m_interpreter.resolved_sections[section][key];
            if (auto* dyna_val_ptr = std::get_if<std::unique_ptr<DynaValue>>(&value.m_value)) {
                (*dyna_val_ptr)->set(new_value);
                const auto& location = m_interpreter.value_locations.at(section).at(key);
                m_dirty_values[section][key] = {new_value, location.line, location.column};
                return;
            } else {
                throw std::runtime_error("Cannot set value: key '" + key + "' in section '" + section + "' is not dynamic.");
            }
        }

        if (m_interpreter.resolved_sections.count(section)) {
            m_interpreter.resolved_sections[section][key] = DynaValue(new_value);
            m_dirty_values[section][key] = {new_value, 0, 0};
            return;
        }

        throw std::runtime_error("Cannot set value: section '" + section + "' does not exist.");
    }

    std::vector<std::unique_ptr<Stmt>> YiniManager::load_file(const std::string& filepath, std::set<std::string>& loaded_files)
    {
        if (loaded_files.count(filepath)) {
            return {};
        }
        loaded_files.insert(filepath);

        std::ifstream file(filepath);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open file: " + filepath);
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string source = buffer.str();

        Lexer lexer(source, filepath);
        std::vector<Token> tokens = lexer.scanTokens();
        Parser parser(tokens);
        std::vector<std::unique_ptr<Stmt>> current_ast = parser.parse();

        std::vector<std::unique_ptr<Stmt>> combined_ast;

        for (auto& stmt : current_ast) {
            if (auto* include_node = dynamic_cast<Include*>(stmt.get())) {
                for (const auto& file_expr : include_node->files) {
                    auto* literal_node = dynamic_cast<Literal*>(file_expr.get());
                    if (literal_node && std::holds_alternative<std::string>(literal_node->value.m_value)) {
                        std::string include_path = std::get<std::string>(literal_node->value.m_value);
                        auto included_ast = load_file(include_path, loaded_files);
                        merge_asts(combined_ast, included_ast);
                    }
                }
            }
        }

        merge_asts(combined_ast, current_ast);

        return combined_ast;
    }

    void YiniManager::save_changes()
    {
        if (m_dirty_values.empty()) {
            return;
        }

        std::ifstream infile(m_filepath);
        if (!infile) {
            throw std::runtime_error("Could not open original file for reading: " + m_filepath);
        }

        std::vector<std::string> lines;
        std::string current_line;
        while (std::getline(infile, current_line)) {
            lines.push_back(current_line);
        }
        infile.close();

        std::map<std::string, bool> new_sections_written;

        for (auto const& [section, keys] : m_dirty_values) {
            for (auto const& [key, dirty_value] : keys) {
                if (dirty_value.line > 0) {
                    std::string& line = lines[dirty_value.line - 1];
                    size_t eq_pos = line.find('=');
                    std::string key_part = line.substr(0, eq_pos + 1);

                    std::string comment_part = "";
                    size_t comment_pos = line.find("//");
                    if (comment_pos != std::string::npos && comment_pos > eq_pos) {
                        comment_part = line.substr(comment_pos);
                    }

                    line = key_part + " " + m_interpreter.stringify(dirty_value.value) + (comment_part.empty() ? "" : " " + comment_part);
                } else {
                    if (!new_sections_written[section]) {
                        lines.push_back("[" + section + "]");
                        new_sections_written[section] = true;
                    }
                    lines.push_back(key + " = " + m_interpreter.stringify(dirty_value.value));
                }
            }
        }

        std::string temp_filepath = m_filepath + ".tmp";
        std::ofstream outfile(temp_filepath, std::ios::trunc);
        if (!outfile) {
            throw std::runtime_error("Could not open temporary file for writing: " + temp_filepath);
        }

        for (const auto& line : lines) {
            outfile << line << "\n";
        }
        outfile.close();

        if (std::rename(temp_filepath.c_str(), m_filepath.c_str()) != 0) {
            std::remove(temp_filepath.c_str());
            throw std::runtime_error("Failed to rename temporary file.");
        }

        m_dirty_values.clear();
    }

    const Schema* YiniManager::get_schema() const
    {
        return m_schema.get();
    }

    void YiniManager::merge_asts(std::vector<std::unique_ptr<Stmt>>& base_ast, std::vector<std::unique_ptr<Stmt>>& new_ast)
    {
        Define* base_define = nullptr;
        std::map<std::string, Section*> base_sections;

        for (auto& stmt : base_ast) {
            if (auto* define = dynamic_cast<Define*>(stmt.get())) {
                base_define = define;
            } else if (auto* section = dynamic_cast<Section*>(stmt.get())) {
                base_sections[section->name.lexeme] = section;
            }
        }

        for (auto& new_stmt_ptr : new_ast) {
            if (!new_stmt_ptr) continue;

            if (auto* new_define = dynamic_cast<Define*>(new_stmt_ptr.get())) {
                if (!base_define) {
                // If base has no define block, create one and add it to the front.
                auto created_define = std::make_unique<Define>(std::vector<std::unique_ptr<KeyValue>>{});
                base_define = created_define.get();
                base_ast.insert(base_ast.begin(), std::move(created_define));
                }

            // Index existing macros for efficient lookup
            std::map<std::string, KeyValue*> existing_macros;
            for (auto& kv : base_define->values) {
                existing_macros[kv->key.lexeme] = kv.get();
            }

            // Merge new macros, overriding existing ones
            for (auto& new_macro_ptr : new_define->values) {
                auto* new_macro = new_macro_ptr.get();
                if (existing_macros.count(new_macro->key.lexeme)) {
                    existing_macros[new_macro->key.lexeme]->value = std::move(new_macro->value);
                } else {
                    existing_macros[new_macro->key.lexeme] = new_macro;
                    base_define->values.push_back(std::move(new_macro_ptr));
                }
                }
            } else if (auto* new_section = dynamic_cast<Section*>(new_stmt_ptr.get())) {
                if (base_sections.count(new_section->name.lexeme)) {
                // Section exists, merge statements
                    auto* existing_section = base_sections[new_section->name.lexeme];

                // Index existing keys for efficient lookup
                std::map<std::string, KeyValue*> existing_kvs;
                for (auto& stmt : existing_section->statements) {
                    if (auto* kv = dynamic_cast<KeyValue*>(stmt.get())) {
                        existing_kvs[kv->key.lexeme] = kv;
                    }
                }

                // Merge new statements, overriding existing key-values
                for (auto& new_section_stmt_ptr : new_section->statements) {
                    if (auto* new_kv = dynamic_cast<KeyValue*>(new_section_stmt_ptr.get())) {
                        if (existing_kvs.count(new_kv->key.lexeme)) {
                            existing_kvs[new_kv->key.lexeme]->value = std::move(new_kv->value);
                        } else {
                            existing_kvs[new_kv->key.lexeme] = new_kv;
                            existing_section->statements.push_back(std::move(new_section_stmt_ptr));
                        }
                    } else {
                        // For other statements like Register (`+=`), just append them
                        existing_section->statements.push_back(std::move(new_section_stmt_ptr));
                    }
                    }
                } else {
                // Section is new, just move the whole thing
                    base_ast.push_back(std::move(new_stmt_ptr));
                // And add it to our index for subsequent merges in the same load operation
                    base_sections[new_section->name.lexeme] = new_section;
                }
            } else if (!dynamic_cast<Include*>(new_stmt_ptr.get())) {
            // For other top-level statements (if any), just append them.
            // We specifically ignore Include nodes as they've already been processed.
                base_ast.push_back(std::move(new_stmt_ptr));
            }
        }
    }
}