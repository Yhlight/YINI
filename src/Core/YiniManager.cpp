#include "YiniManager.h"
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <functional>
#include <set>
#include <map>

namespace YINI
{
    // Helper to parse a single file and return its AST.
    static std::vector<std::unique_ptr<Stmt>> parse_file(const std::string& filepath)
    {
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
        return parser.parse();
    }

    YiniManager::YiniManager() = default;

    void YiniManager::load(const std::string& filepath)
    {
        m_filepath = filepath;
        m_schema = nullptr;
        m_ast.clear();

        std::set<std::string> loaded_files;
        m_ast = load_file_recursive(filepath, loaded_files);

        // Post-processing to separate the schema from the main AST
        for (auto it = m_ast.begin(); it != m_ast.end(); ) {
            if (auto* schema = dynamic_cast<Schema*>(it->get())) {
                if (!m_schema) { // Take the first schema found
                    m_schema.reset(static_cast<Schema*>(it->release()));
                }
                it = m_ast.erase(it);
            } else {
                ++it;
            }
        }

        m_interpreter.interpret(m_ast);
    }

    std::vector<std::unique_ptr<Stmt>> YiniManager::load_file_recursive(const std::string& filepath, std::set<std::string>& loaded_files)
    {
        if (loaded_files.count(filepath)) {
            return {}; // Prevent circular includes
        }
        loaded_files.insert(filepath);

        auto current_ast = parse_file(filepath);
        std::vector<std::unique_ptr<Stmt>> merged_ast;

        // 1. Recursively load and merge includes first to establish the base
        for(const auto& stmt : current_ast) {
            if (auto* include_node = dynamic_cast<Include*>(stmt.get())) {
                for (const auto& file_expr : include_node->files) {
                    if(const auto* literal = dynamic_cast<const Literal*>(file_expr.get())) {
                        if(std::holds_alternative<std::string>(literal->value.m_value)) {
                            std::string include_path = std::get<std::string>(literal->value.m_value);
                            auto included_ast = load_file_recursive(include_path, loaded_files);
                            merge_asts(merged_ast, included_ast);
                        }
                    }
                }
            }
        }

        // 2. Then, merge the statements from the current file on top
        merge_asts(merged_ast, current_ast);
        return merged_ast;
    }

    void YiniManager::save_changes()
    {
        std::stringstream ss;
        for (const auto& stmt : m_ast)
        {
            if (const auto* define_node = dynamic_cast<const Define*>(stmt.get()))
            {
                ss << "[#define]\n";
                for(const auto& kv : define_node->values) {
                    if (!kv->doc_comment.empty()) ss << "//" << kv->doc_comment << "\n";
                    ss << kv->key.lexeme << " = " << m_interpreter.stringify(kv->value->accept(m_interpreter));
                    if (!kv->inline_comment.empty()) ss << " //" << kv->inline_comment;
                    ss << "\n";
                }
            }
            else if (const auto* include_node = dynamic_cast<const Include*>(stmt.get()))
            {
                ss << "[#include]\n";
                for (const auto& file_expr : include_node->files) {
                     if(const auto* literal = dynamic_cast<const Literal*>(file_expr.get())) {
                        ss << "+= " << m_interpreter.stringify(literal->value) << "\n";
                    }
                }
            }
            else if (const auto* section_node = dynamic_cast<const Section*>(stmt.get()))
            {
                if (!section_node->doc_comment.empty()) {
                    ss << "//" << section_node->doc_comment << "\n";
                }
                ss << "[" << section_node->name.lexeme << "]\n";

                for (const auto& key_stmt : section_node->statements)
                {
                    if (const auto* kv_node = dynamic_cast<const KeyValue*>(key_stmt.get()))
                    {
                        if (!kv_node->doc_comment.empty()) {
                            ss << "//" << kv_node->doc_comment << "\n";
                        }

                        YiniValue value = get_value(section_node->name.lexeme, kv_node->key.lexeme);
                        ss << kv_node->key.lexeme << " = " << m_interpreter.stringify(value);

                        if (!kv_node->inline_comment.empty()) {
                            ss << " //" << kv_node->inline_comment;
                        }
                        ss << "\n";
                    }
                }
            }
        }

        std::ofstream outfile(m_filepath, std::ios::trunc);
        if (!outfile) {
            throw std::runtime_error("Could not open file for writing: " + m_filepath);
        }
        outfile << ss.str();
        outfile.close();

        m_dirty_values.clear();
    }

    const Interpreter& YiniManager::get_interpreter() const
    {
        return m_interpreter;
    }

    YiniValue YiniManager::get_value(const std::string& section, const std::string& key)
    {
        if (m_interpreter.resolved_sections.count(section) && m_interpreter.resolved_sections.at(section).count(key)) {
            YiniValue& value = m_interpreter.resolved_sections.at(section).at(key);
            if (auto* dyna_val_ptr = std::get_if<std::unique_ptr<DynaValue>>(&value.m_value)) {
                return (*dyna_val_ptr)->get();
            }
            return value;
        }
        throw std::runtime_error("Value not found for section '" + section + "' and key '" + key + "'.");
    }

    void YiniManager::set_value(const std::string& section, const std::string& key, YiniValue new_value)
    {
        if (m_interpreter.resolved_sections.count(section) && m_interpreter.resolved_sections.at(section).count(key)) {
            YiniValue& value = m_interpreter.resolved_sections.at(section).at(key);
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

    const Schema* YiniManager::get_schema() const
    {
        return m_schema.get();
    }

    bool YiniManager::validate()
    {
        m_last_validation_errors.clear();
        if (!m_schema) {
            return true;
        }

        Validator validator;
        m_last_validation_errors = validator.validate(*m_schema, m_interpreter);

        return m_last_validation_errors.empty();
    }

    std::string YiniManager::get_section_doc_comment(const std::string& section) const
    {
        for (const auto& stmt : m_ast)
        {
            if (const auto* section_node = dynamic_cast<const Section*>(stmt.get()))
            {
                if (section_node->name.lexeme == section)
                {
                    return section_node->doc_comment;
                }
            }
        }
        return "";
    }

    std::string YiniManager::get_key_doc_comment(const std::string& section, const std::string& key) const
    {
        for (const auto& stmt : m_ast)
        {
            if (const auto* section_node = dynamic_cast<const Section*>(stmt.get()))
            {
                if (section_node->name.lexeme == section)
                {
                    for (const auto& key_stmt : section_node->statements)
                    {
                        if (const auto* kv_node = dynamic_cast<const KeyValue*>(key_stmt.get()))
                        {
                            if (kv_node->key.lexeme == key)
                            {
                                return kv_node->doc_comment;
                            }
                        }
                    }
                }
            }
        }
        return "";
    }

    std::string YiniManager::get_key_inline_comment(const std::string& section, const std::string& key) const
    {
        for (const auto& stmt : m_ast)
        {
            if (const auto* section_node = dynamic_cast<const Section*>(stmt.get()))
            {
                if (section_node->name.lexeme == section)
                {
                    for (const auto& key_stmt : section_node->statements)
                    {
                        if (const auto* kv_node = dynamic_cast<const KeyValue*>(key_stmt.get()))
                        {
                            if (kv_node->key.lexeme == key)
                            {
                                return kv_node->inline_comment;
                            }
                        }
                    }
                }
            }
        }
        return "";
    }

    void YiniManager::set_section_doc_comment(const std::string& section, const std::string& comment)
    {
        for (const auto& stmt : m_ast)
        {
            if (auto* section_node = dynamic_cast<Section*>(stmt.get()))
            {
                if (section_node->name.lexeme == section)
                {
                    section_node->doc_comment = comment;
                    return;
                }
            }
        }
    }

    void YiniManager::set_key_doc_comment(const std::string& section, const std::string& key, const std::string& comment)
    {
        for (const auto& stmt : m_ast)
        {
            if (auto* section_node = dynamic_cast<Section*>(stmt.get()))
            {
                if (section_node->name.lexeme == section)
                {
                    for (auto& key_stmt : section_node->statements)
                    {
                        if (auto* kv_node = dynamic_cast<KeyValue*>(key_stmt.get()))
                        {
                            if (kv_node->key.lexeme == key)
                            {
                                kv_node->doc_comment = comment;
                                return;
                            }
                        }
                    }
                }
            }
        }
    }

    void YiniManager::set_key_inline_comment(const std::string& section, const std::string& key, const std::string& comment)
    {
        for (const auto& stmt : m_ast)
        {
            if (auto* section_node = dynamic_cast<Section*>(stmt.get()))
            {
                if (section_node->name.lexeme == section)
                {
                    for (auto& key_stmt : section_node->statements)
                    {
                        if (auto* kv_node = dynamic_cast<KeyValue*>(key_stmt.get()))
                        {
                            if (kv_node->key.lexeme == key)
                            {
                                kv_node->inline_comment = comment;
                                return;
                            }
                        }
                    }
                }
            }
        }
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
                    base_ast.insert(base_ast.begin(), std::make_unique<Define>(std::vector<std::unique_ptr<KeyValue>>{}));
                    base_define = dynamic_cast<Define*>(base_ast.front().get());
                }

                std::map<std::string, KeyValue*> existing_macros;
                for (auto& kv : base_define->values) {
                    existing_macros[kv->key.lexeme] = kv.get();
                }

                for (auto& new_macro_ptr : new_define->values) {
                    if (existing_macros.count(new_macro_ptr->key.lexeme)) {
                        existing_macros[new_macro_ptr->key.lexeme]->value = std::move(new_macro_ptr->value);
                    } else {
                        base_define->values.push_back(std::move(new_macro_ptr));
                    }
                }
            } else if (auto* new_section = dynamic_cast<Section*>(new_stmt_ptr.get())) {
                if (base_sections.count(new_section->name.lexeme)) {
                    auto* existing_section = base_sections[new_section->name.lexeme];
                    std::map<std::string, KeyValue*> existing_kvs;
                     for (auto& stmt : existing_section->statements) {
                        if (auto* kv = dynamic_cast<KeyValue*>(stmt.get())) {
                            existing_kvs[kv->key.lexeme] = kv;
                        }
                    }
                    for (auto& new_section_stmt_ptr : new_section->statements) {
                        if (auto* new_kv = dynamic_cast<KeyValue*>(new_section_stmt_ptr.get())) {
                            if (existing_kvs.count(new_kv->key.lexeme)) {
                                existing_kvs[new_kv->key.lexeme]->value = std::move(new_kv->value);
                            } else {
                                existing_section->statements.push_back(std::move(new_section_stmt_ptr));
                            }
                        } else {
                            existing_section->statements.push_back(std::move(new_section_stmt_ptr));
                        }
                    }
                } else {
                    base_ast.push_back(std::move(new_stmt_ptr));
                    base_sections[new_section->name.lexeme] = dynamic_cast<Section*>(base_ast.back().get());
                }
            } else {
                 base_ast.push_back(std::move(new_stmt_ptr));
            }
        }
    }
}