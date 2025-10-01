#include "YiniManager.h"
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include <fstream>
#include <sstream>
#include <iostream>

namespace YINI
{
    YiniManager::YiniManager() = default;

    void YiniManager::load(const std::string& filepath)
    {
        std::set<std::string> loaded_files;
        auto final_ast = load_file(filepath, loaded_files);
        interpreter.interpret(final_ast);
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