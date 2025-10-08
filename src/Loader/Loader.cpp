#include "Loader.h"
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "Resolver/Resolver.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <set>
#include <algorithm>
#include <vector>

namespace Yini
{

// Helper function to get the directory part of a filepath
std::string getDirectory(const std::string& filepath) {
    size_t last_slash_idx = filepath.rfind('/');
    if (std::string::npos != last_slash_idx) {
        return filepath.substr(0, last_slash_idx + 1); // Keep the slash
    }
    return "";
}

std::vector<std::unique_ptr<SectionNode>> Loader::load(const std::string& filepath) {
    std::vector<std::string> loaded_files; // To prevent circular includes
    auto finalAst = parseFile(filepath, loaded_files);

    // The resolver should be called only once, at the very top level of the load call.
    Yini::Resolver resolver(finalAst);
    resolver.resolve();

    return finalAst;
}

std::vector<std::unique_ptr<SectionNode>> Loader::parseFile(const std::string& filepath, std::vector<std::string>& loaded_files) {
    if (std::find(loaded_files.begin(), loaded_files.end(), filepath) != loaded_files.end()) {
        // Circular dependency detected, return empty AST to break the loop.
        return {};
    }
    loaded_files.push_back(filepath);

    std::ifstream file(filepath);
    if (!file) {
        throw std::runtime_error("Could not open file: " + filepath);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    Yini::Lexer lexer(source);
    std::vector<Yini::Token> tokens = lexer.scanTokens();
    Yini::Parser parser(tokens);
    auto ast = parser.parse();

    // This vector will hold the AST from included files
    std::vector<std::unique_ptr<SectionNode>> includedAst;

    // This vector will hold the sections from the current file
    std::vector<std::unique_ptr<SectionNode>> currentFileAst;

    for (auto& section : ast) {
        if (section->special_type == SpecialSectionType::Include) {
            for (const auto& pair : section->pairs) {
                // The "key" is the `+=` token, the value is the file path
                auto* pathValueNode = dynamic_cast<StringValue*>(pair->value.get());
                 if (!pathValueNode) {
                    // For now, we only support string literals for include paths for simplicity
                    continue;
                }
                std::string include_path = getDirectory(filepath) + pathValueNode->value;
                auto temp_ast = parseFile(include_path, loaded_files);
                // Move contents of temp_ast into includedAst
                includedAst.insert(includedAst.end(), std::make_move_iterator(temp_ast.begin()), std::make_move_iterator(temp_ast.end()));
            }
        } else {
             currentFileAst.push_back(std::move(section));
        }
    }

    // The final AST for this file is the included AST with the current file's AST merged on top.
    mergeAst(includedAst, currentFileAst);

    return includedAst;
}

// Overload for merging a single section
void Loader::mergeAst(std::vector<std::unique_ptr<SectionNode>>& base, std::unique_ptr<SectionNode>& section_to_merge) {
    if (!section_to_merge) return;

    SectionNode* target_section = nullptr;
    for (auto& base_section : base) {
        if (base_section->name.lexeme == section_to_merge->name.lexeme) {
            target_section = base_section.get();
            break;
        }
    }

    if (target_section) {
        // Merge key-value pairs. Later files (or sections in the same file) override earlier ones.
        for (auto& pair_to_merge : section_to_merge->pairs) {
             bool key_exists = false;
             for(auto& base_pair : target_section->pairs) {
                 if (base_pair->key.lexeme == pair_to_merge->key.lexeme) {
                     base_pair->value = std::move(pair_to_merge->value);
                     key_exists = true;
                     break;
                 }
             }
             if (!key_exists) {
                 target_section->pairs.push_back(std::move(pair_to_merge));
             }
        }
    } else {
        base.push_back(std::move(section_to_merge));
    }
}


void Loader::mergeAst(std::vector<std::unique_ptr<SectionNode>>& base, std::vector<std::unique_ptr<SectionNode>>& to_merge) {
    for (auto& section_to_merge : to_merge) {
        mergeAst(base, section_to_merge);
    }
}

} // namespace Yini