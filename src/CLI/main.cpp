#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "Resolver/Resolver.h"
#include "Ymeta/YmetaManager.h"
#include "Validator/Validator.h"

static void run_file(const char* path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file '" << path << "'" << std::endl;
        return;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    try {
        auto ast = parser.parse();
        YINI::YmetaManager ymeta_manager;
        ymeta_manager.load(path);
        YINI::Resolver resolver(ast, ymeta_manager);
        auto nested_config = resolver.resolve();

        // Flatten the map for the validator
        std::map<std::string, std::any> flat_config;
        for (const auto& [section_name, section_map] : nested_config) {
            if (section_name.empty()) {
                for (const auto& [key, value] : section_map) {
                    flat_config[key] = value;
                }
            } else {
                for (const auto& [key, value] : section_map) {
                    flat_config[section_name + "." + key] = value;
                }
            }
        }

        YINI::Validator validator(flat_config, ast);
        validator.validate();
        ymeta_manager.save(path);
        std::cout << "Validation completed successfully." << std::endl;
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

static void run_prompt() {
    std::string line;
    YINI::YmetaManager ymeta_manager; // A single manager for the REPL session
    for (;;) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) {
            std::cout << std::endl;
            break;
        }
        if (line.empty()) continue;

        YINI::Lexer lexer(line);
        auto tokens = lexer.scan_tokens();
        YINI::Parser parser(tokens);
        try {
            auto ast = parser.parse();
            // Note: .ymeta load/save doesn't make sense for REPL without a file context
            YINI::Resolver resolver(ast, ymeta_manager);
            auto nested_config = resolver.resolve();

            // Flatten the map for the validator
            std::map<std::string, std::any> flat_config;
            for (const auto& [section_name, section_map] : nested_config) {
                if (section_name.empty()) {
                    for (const auto& [key, value] : section_map) {
                        flat_config[key] = value;
                    }
                } else {
                    for (const auto& [key, value] : section_map) {
                        flat_config[section_name + "." + key] = value;
                    }
                }
            }

            YINI::Validator validator(flat_config, ast);
            validator.validate();
             std::cout << "Validation completed successfully." << std::endl;
        } catch (const std::runtime_error& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc > 2) {
        std::cout << "Usage: yini [script]" << std::endl;
        return 64; // EX_USAGE
    } else if (argc == 2) {
        run_file(argv[1]);
    } else {
        run_prompt();
    }

    return 0;
}