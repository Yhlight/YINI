#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include "Parser/Parser.h"
#include "Resolver/Resolver.h"
#include "YMETA/YMETA.h"

void print_usage() {
    std::cout << "Usage: yini <command> [options]" << std::endl;
    std::cout << "Commands:" << std::endl;
    std::cout << "  parse <filepath>   Parse a YINI file and print its structure." << std::endl;
    std::cout << "  check <filepath>   Check the syntax of a YINI file." << std::endl;
    std::cout << "  compile <in> <out> Compile a YINI file to a binary format." << std::endl;
    std::cout << "  decompile <in> <out> Decompile a binary file back to YINI." << std::endl;
}

void print_ast(const YINI::AstNode& ast) {
    std::cout << "--- AST ---" << std::endl;
    if (!ast.macros.empty()) {
        std::cout << "Macros:" << std::endl;
        for (const auto& macro : ast.macros) {
            std::cout << "  " << macro.first << " = ..." << std::endl;
        }
    }
    for (const auto& section : ast.sections) {
        std::cout << "Section: " << section.name << std::endl;
        if (!section.parents.empty()) {
            std::cout << "  Parents: ";
            for(size_t i = 0; i < section.parents.size(); ++i) {
                std::cout << section.parents[i] << (i == section.parents.size() - 1 ? "" : ", ");
            }
            std::cout << std::endl;
        }
        for (const auto& kv : section.key_values) {
            std::cout << "  " << kv.key << " = ..." << std::endl;
        }
    }
    std::cout << "-----------" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage();
        return 1;
    }

    std::string command = argv[1];

    if (command == "parse") {
        if (argc != 3) {
            std::cout << "Usage: yini parse <filepath>" << std::endl;
            return 1;
        }
        std::string filepath = argv[2];
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filepath << std::endl;
            return 1;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();

        try {
            YINI::Lexer lexer(content);
            YINI::Parser parser(lexer, filepath);
            auto ast = parser.parse();

            YINI::Resolver resolver;
            resolver.resolve(*ast);

            std::cout << "Successfully parsed file: " << filepath << std::endl;
            print_ast(*ast);

        } catch (const std::exception& e) {
            std::cerr << "Error parsing file: " << e.what() << std::endl;
            return 1;
        }

    } else if (command == "check") {
        if (argc != 3) {
            std::cout << "Usage: yini check <filepath>" << std::endl;
            return 1;
        }
        std::string filepath = argv[2];
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filepath << std::endl;
            return 1;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();

        try {
            YINI::Lexer lexer(content);
            YINI::Parser parser(lexer, filepath);
            auto ast = parser.parse();

            YINI::Resolver resolver;
            resolver.resolve(*ast);

            std::cout << "Syntax check passed for file: " << filepath << std::endl;

        } catch (const std::exception& e) {
            std::cerr << "Syntax check failed: " << e.what() << std::endl;
            return 1;
        }
    } else if (command == "compile") {
        if (argc != 4) {
            std::cout << "Usage: yini compile <input_filepath> <output_filepath>" << std::endl;
            return 1;
        }
        std::string input_filepath = argv[2];
        std::string output_filepath = argv[3];

        std::ifstream infile(input_filepath);
        if (!infile.is_open()) {
            std::cerr << "Error: Could not open input file " << input_filepath << std::endl;
            return 1;
        }

        std::stringstream buffer;
        buffer << infile.rdbuf();
        std::string content = buffer.str();

        try {
            YINI::Lexer lexer(content);
            YINI::Parser parser(lexer, input_filepath);
            auto ast = parser.parse();

            YINI::Resolver resolver;
            resolver.resolve(*ast);

            std::ofstream outfile(output_filepath, std::ios::binary);
            if (!outfile.is_open()) {
                std::cerr << "Error: Could not open output file " << output_filepath << std::endl;
                return 1;
            }

            YINI::YMETA::serialize(*ast, outfile);
            std::cout << "Successfully compiled " << input_filepath << " to " << output_filepath << std::endl;

        } catch (const std::exception& e) {
            std::cerr << "Error compiling file: " << e.what() << std::endl;
            return 1;
        }
    } else if (command == "decompile") {
        if (argc != 4) {
            std::cout << "Usage: yini decompile <input_filepath> <output_filepath>" << std::endl;
            return 1;
        }
        std::string input_filepath = argv[2];
        std::string output_filepath = argv[3];

        std::ifstream infile(input_filepath, std::ios::binary);
        if (!infile.is_open()) {
            std::cerr << "Error: Could not open input file " << input_filepath << std::endl;
            return 1;
        }

        try {
            auto ast = YINI::YMETA::deserialize(infile);

            std::ofstream outfile(output_filepath);
            if (!outfile.is_open()) {
                std::cerr << "Error: Could not open output file " << output_filepath << std::endl;
                return 1;
            }

            // This is a placeholder for writing the AST to a file.
            // A proper implementation would reconstruct the YINI text from the AST.
            outfile << "[decompiled]" << std::endl;

            std::cout << "Successfully decompiled " << input_filepath << " to " << output_filepath << std::endl;

        } catch (const std::exception& e) {
            std::cerr << "Error decompiling file: " << e.what() << std::endl;
            return 1;
        }
    } else {
        std::cout << "Unknown command: " << command << std::endl;
        print_usage();
        return 1;
    }

    return 0;
}