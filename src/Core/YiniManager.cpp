#include "YiniManager.h"
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <regex>
#include <cstdio>

namespace YINI
{
    YiniManager::YiniManager() = default;

    void YiniManager::load(const std::string& filepath)
    {
        m_filepath = filepath;
        std::set<std::string> loaded_files;
        auto final_ast = load_file(filepath, loaded_files);
        interpreter.interpret(final_ast);
    }

    std::any YiniManager::get_value(const std::string& section, const std::string& key)
    {
        if (interpreter.resolved_sections.count(section) && interpreter.resolved_sections[section].count(key)) {
            std::any& value = interpreter.resolved_sections[section][key];
            if (value.type() == typeid(DynaValue)) {
                return std::any_cast<DynaValue&>(value).get();
            }
            return value;
        }
        throw std::runtime_error("Value not found for section '" + section + "' and key '" + key + "'.");
    }

    void YiniManager::set_value(const std::string& section, const std::string& key, std::any new_value)
    {
        if (interpreter.resolved_sections.count(section) && interpreter.resolved_sections[section].count(key)) {
            std::any& value = interpreter.resolved_sections[section][key];
            if (value.type() == typeid(DynaValue)) {
                std::any_cast<DynaValue&>(value).set(new_value);
                m_dirty_values[section][key] = new_value;
                return;
            }
        }
        throw std::runtime_error("Cannot set value: key '" + key + "' in section '" + section + "' is not dynamic or does not exist.");
    }

    std::vector<std::unique_ptr<Stmt>> YiniManager::load_file(const std::string& filepath, std::set<std::string>& loaded_files)
    {
        if (loaded_files.count(filepath)) {
            // In a real implementation, might want to warn or just ignore. For now, ignore.
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

        Lexer lexer(source);
        std::vector<Token> tokens = lexer.scanTokens();
        Parser parser(tokens);
        std::vector<std::unique_ptr<Stmt>> current_ast = parser.parse();

        std::vector<std::unique_ptr<Stmt>> combined_ast;

        // Find and process include statements first
        for (auto& stmt : current_ast) {
            if (auto* include_node = dynamic_cast<Include*>(stmt.get())) {
                for (const auto& file_expr : include_node->files) {
                    auto* literal_node = dynamic_cast<Literal*>(file_expr.get());
                    if (literal_node && literal_node->value.type() == typeid(std::string)) {
                        std::string include_path = std::any_cast<std::string>(literal_node->value);
                        auto included_ast = load_file(include_path, loaded_files);
                        merge_asts(combined_ast, included_ast);
                    }
                }
            }
        }

        // Merge the current file's AST after all includes
        merge_asts(combined_ast, current_ast);

        return combined_ast;
    }

    void YiniManager::save_changes()
    {
        if (m_dirty_values.empty()) {
            return; // Nothing to save
        }

        std::ifstream infile(m_filepath);
        if (!infile) {
            throw std::runtime_error("Could not open original file for reading: " + m_filepath);
        }

        std::string temp_filepath = m_filepath + ".tmp";
        std::ofstream outfile(temp_filepath, std::ios::trunc);
        if (!outfile) {
            throw std::runtime_error("Could not open temporary file for writing: " + temp_filepath);
        }

        std::string line;
        std::string current_section;
        std::regex section_regex(R"(\s*\[([^\]]+)\]\s*)");

        while (std::getline(infile, line)) {
            std::smatch match;
            if (std::regex_match(line, match, section_regex)) {
                current_section = match[1].str();
            } else {
                size_t eq_pos = line.find('=');
                if (eq_pos != std::string::npos) {
                    std::string key = line.substr(0, eq_pos);
                    key.erase(0, key.find_first_not_of(" \t\n\r"));
                    key.erase(key.find_last_not_of(" \t\n\r") + 1);

                    if (!current_section.empty() && m_dirty_values.count(current_section) && m_dirty_values[current_section].count(key)) {
                        std::string new_value_str = interpreter.stringify(m_dirty_values[current_section][key]);

                        std::string comment_part = "";
                        size_t comment_pos = line.find("//");
                        if (comment_pos != std::string::npos && comment_pos > eq_pos) {
                            comment_part = line.substr(comment_pos);
                        }

                        std::string key_part = line.substr(0, eq_pos + 1);
                        line = key_part + " " + new_value_str + (comment_part.empty() ? "" : " " + comment_part);

                        // Mark as written
                        m_dirty_values[current_section].erase(key);
                        if (m_dirty_values[current_section].empty()) {
                            m_dirty_values.erase(current_section);
                        }
                    }
                }
            }
            outfile << line << "\n";
        }

        infile.close();

        // Append any new values that were not in the original file
        for (const auto& section_pair : m_dirty_values) {
            outfile << "[" << section_pair.first << "]\n";
            for (const auto& key_pair : section_pair.second) {
                outfile << key_pair.first << " = " << interpreter.stringify(key_pair.second) << "\n";
            }
        }

        outfile.close();

        if (std::rename(temp_filepath.c_str(), m_filepath.c_str()) != 0) {
            std::remove(temp_filepath.c_str());
            throw std::runtime_error("Failed to rename temporary file.");
        }

        m_dirty_values.clear();
    }

    void YiniManager::merge_asts(std::vector<std::unique_ptr<Stmt>>& base_ast, std::vector<std::unique_ptr<Stmt>>& new_ast)
    {
        Define* base_define = nullptr;
        std::map<std::string, Section*> base_sections;

        // First, index the base AST
        for (auto& stmt : base_ast) {
            if (auto* define = dynamic_cast<Define*>(stmt.get())) {
                base_define = define;
            } else if (auto* section = dynamic_cast<Section*>(stmt.get())) {
                base_sections[section->name.lexeme] = section;
            }
        }

        // Now, merge the new AST into the base
        for (auto& new_stmt_ptr : new_ast) {
            if (!new_stmt_ptr) continue;

            if (auto* new_define = dynamic_cast<Define*>(new_stmt_ptr.get())) {
                if (!base_define) {
                    // If base has no define block, create one and move it to the front
                    base_ast.insert(base_ast.begin(), std::make_unique<Define>(std::vector<std::unique_ptr<KeyValue>>{}));
                    base_define = dynamic_cast<Define*>(base_ast[0].get());
                }
                // Move all key-values from the new define block to the base one
                for (auto& kv : new_define->values) {
                    base_define->values.push_back(std::move(kv));
                }
            } else if (auto* new_section = dynamic_cast<Section*>(new_stmt_ptr.get())) {
                if (base_sections.count(new_section->name.lexeme)) {
                    // Section exists, merge statements
                    auto* existing_section = base_sections[new_section->name.lexeme];
                    for (auto& s : new_section->statements) {
                        existing_section->statements.push_back(std::move(s));
                    }
                } else {
                    // Section is new, move the whole statement
                    base_ast.push_back(std::move(new_stmt_ptr));
                    base_sections[new_section->name.lexeme] = new_section;
                }
            } else if (!dynamic_cast<Include*>(new_stmt_ptr.get())) {
                // If it's not a define, section, or include, just append it
                base_ast.push_back(std::move(new_stmt_ptr));
            }
        }
    }
}