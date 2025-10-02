#include "YiniManager.h"
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "Core/YiniException.h"
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
        std::ifstream file(filepath);
        if (!file.is_open()) {
            throw YiniException("Could not open file", filepath, 0, 0);
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        load_from_string(buffer.str(), filepath);
    }

    void YiniManager::load_from_string(const std::string& content, const std::string& filepath) {
        m_filepath = filepath;
        m_lines.clear();
        std::stringstream ss(content);
        std::string current_line;
        while (std::getline(ss, current_line)) { m_lines.push_back(current_line); }
        std::set<std::string> loaded_files;
        auto final_ast = load_file(filepath, loaded_files, content);
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
        throw YiniException("Value not found for section '" + section + "' and key '" + key + "'.", m_filepath, 0, 0);
    }

    void YiniManager::set_value(const std::string& section, const std::string& key, std::any new_value)
    {
        // Case 1: The key already exists.
        if (interpreter.resolved_sections.count(section) && interpreter.resolved_sections[section].count(key)) {
            std::any& value = interpreter.resolved_sections[section][key];
            if (value.type() == typeid(DynaValue)) {
                std::any_cast<DynaValue&>(value).set(new_value);
                const auto& location = interpreter.value_locations.at(section).at(key);
                m_dirty_values[section][key] = {new_value, location.line, location.column};
                return;
            } else {
                // Key exists but is not dynamic.
                const auto& location = interpreter.value_locations.at(section).at(key);
                throw YiniException("Cannot set value: key '" + key + "' in section '" + section + "' is not dynamic.", m_filepath, location.line, location.column);
            }
        }

        // Case 2: The key does not exist, but the section does. Add a new value.
        if (interpreter.resolved_sections.count(section)) {
            interpreter.resolved_sections[section][key] = DynaValue(new_value);
            m_dirty_values[section][key] = {new_value, 0, 0}; // line 0 indicates a new value to be appended.
            return;
        }

        // Case 3: The section does not exist.
        throw YiniException("Cannot set value: section '" + section + "' does not exist.", m_filepath, 0, 0);
    }

    std::vector<std::unique_ptr<Stmt>> YiniManager::load_file(const std::string& filepath, std::set<std::string>& loaded_files, const std::optional<std::string>& content)
    {
        if (!filepath.empty() && loaded_files.count(filepath)) {
            return {};
        }
        if (!filepath.empty()) {
            loaded_files.insert(filepath);
        }

        std::string source;
        if (content) {
            source = *content;
        } else {
            if (filepath.empty()) {
                // This case should ideally not be reached if called from public methods
                throw YiniException("Cannot load from empty filepath and no content.", "", 0, 0);
            }
            std::ifstream file(filepath);
            if (!file.is_open()) {
                throw YiniException("Could not open file", filepath, 0, 0);
            }
            std::stringstream buffer;
            buffer << file.rdbuf();
            source = buffer.str();
        }

        Lexer lexer(source);
        std::vector<Token> tokens = lexer.scanTokens();
        Parser parser(tokens);
        std::vector<std::unique_ptr<Stmt>> current_ast;
        try {
            current_ast = parser.parse();
        } catch (YiniException& e) {
            e.set_filepath(filepath);
            throw;
        }

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

    void YiniManager::update_line(std::string& line, const DirtyValue& dirty_value)
    {
        size_t eq_pos = line.find('=');
        std::string key_part = line.substr(0, eq_pos + 1);

        std::string comment_part = "";
        size_t comment_pos = line.find("//");
        if (comment_pos != std::string::npos && comment_pos > eq_pos) {
            comment_part = line.substr(comment_pos);
        }

        line = key_part + " " + interpreter.stringify(dirty_value.value) + (comment_part.empty() ? "" : " " + comment_part);
    }

    void YiniManager::save_changes()
    {
        if (m_dirty_values.empty()) {
            return; // Nothing to save
        }

        std::map<std::string, bool> new_sections_written;

        for (auto const& [section, keys] : m_dirty_values) {
            for (auto const& [key, dirty_value] : keys) {
                if (dirty_value.line > 0) { // Existing value
                    update_line(m_lines[dirty_value.line - 1], dirty_value);
                } else { // New value
                    if (!new_sections_written[section]) {
                        m_lines.push_back("[" + section + "]");
                        new_sections_written[section] = true;
                    }
                    m_lines.push_back(key + " = " + interpreter.stringify(dirty_value.value));
                }
            }
        }

        std::string temp_filepath = m_filepath + ".tmp";
        std::ofstream outfile(temp_filepath, std::ios::trunc);
        if (!outfile) {
            throw YiniException("Could not open temporary file for writing", temp_filepath, 0, 0);
        }

        for (const auto& line : m_lines) {
            outfile << line << "\n";
        }
        outfile.close();

        if (std::rename(temp_filepath.c_str(), m_filepath.c_str()) != 0) {
            std::remove(temp_filepath.c_str());
            throw YiniException("Failed to rename temporary file", temp_filepath, 0, 0);
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