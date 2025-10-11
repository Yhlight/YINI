#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "Resolver/Resolver.h"
#include "Ymeta/YmetaManager.h"
#include "Validator/Validator.h"
#include "Cooker.h"

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
        auto resolved_config = resolver.resolve();
        YINI::Validator validator(resolved_config, ast);
        validator.validate();
        ymeta_manager.save(path);
        std::cout << "Validation completed successfully." << std::endl;
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

// Forward declaration for the new run_file overload
static void run_file(const char* path, std::map<std::string, std::any>& config_context, YINI::YmetaManager& ymeta_manager);

static void run_prompt() {
    std::string line;
    YINI::YmetaManager ymeta_manager;
    std::map<std::string, std::any> config_context;

    for (;;) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) {
            std::cout << std::endl;
            break;
        }
        if (line.empty()) continue;

        if (line.rfind(".load ", 0) == 0) {
            std::string path = line.substr(6);
            config_context.clear();
            ymeta_manager = YINI::YmetaManager();
            run_file(path.c_str(), config_context, ymeta_manager);
        } else if (line.rfind(".get ", 0) == 0) {
            std::string key = line.substr(5);
            if (config_context.count(key)) {
                // This is a simplified output. A real implementation would handle different types.
                try {
                    if (config_context[key].type() == typeid(double)) {
                        std::cout << std::any_cast<double>(config_context[key]) << std::endl;
                    } else if (config_context[key].type() == typeid(std::string)) {
                        std::cout << std::any_cast<std::string>(config_context[key]) << std::endl;
                    } else if (config_context[key].type() == typeid(bool)) {
                        std::cout << (std::any_cast<bool>(config_context[key]) ? "true" : "false") << std::endl;
                    } else {
                         std::cout << "[complex type]" << std::endl;
                    }
                } catch (const std::bad_any_cast& e) {
                     std::cerr << "Error: Could not cast value." << std::endl;
                }
            } else {
                std::cout << "null" << std::endl;
            }
        } else if (line == ".exit" || line == ".quit") {
            break;
        }
        else
        {
             try {
                YINI::Lexer lexer(line);
                auto tokens = lexer.scan_tokens();
                YINI::Parser parser(tokens);
                auto ast = parser.parse();
                YINI::Resolver resolver(ast, ymeta_manager);
                auto temp_config = resolver.resolve();
                // We don't merge this into the main context, just validate it.
                YINI::Validator validator(temp_config, ast);
                validator.validate();
                std::cout << "Snippet validated successfully." << std::endl;
            } catch (const std::runtime_error& e) {
                std::cerr << "Error: " << e.what() << std::endl;
            }
        }
    }
}

// Overload run_file to work with an existing context for the REPL
static void run_file(const char* path, std::map<std::string, std::any>& config_context, YINI::YmetaManager& ymeta_manager) {
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
        ymeta_manager.load(path);
        YINI::Resolver resolver(ast, ymeta_manager);
        config_context = resolver.resolve(); // Load into the provided context
        YINI::Validator validator(config_context, ast);
        validator.validate();
        ymeta_manager.save(path);
        std::cout << "File '" << path << "' loaded and validated." << std::endl;
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

static void run_cook(int argc, char* argv[]) {
    std::string output_path;
    std::vector<std::string> input_paths;

    // Basic argument parsing
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-o" && i + 1 < argc) {
            output_path = argv[++i];
        } else {
            input_paths.push_back(arg);
        }
    }

    if (output_path.empty() || input_paths.empty()) {
        std::cerr << "Usage: yini cook -o <output.ybin> <input1.yini> [input2.yini]..." << std::endl;
        return;
    }

    std::vector<std::unique_ptr<YINI::AST::Stmt>> combined_ast;
    YINI::YmetaManager ymeta_manager; // Resolver needs this, though we don't use its features here

    for (const auto& path : input_paths) {
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
        auto ast = parser.parse();
        for (auto& stmt : ast) {
            combined_ast.push_back(std::move(stmt));
        }
    }

    try {
        YINI::Resolver resolver(combined_ast, ymeta_manager);
        auto resolved_config = resolver.resolve();

        // We don't need to validate here, as cooking is a build-time step.
        // The final validation should happen on the loaded data if needed.

        YINI::Cooker cooker;
        cooker.cook(resolved_config, output_path);

    } catch (const std::runtime_error& e) {
        std::cerr << "Error during cooking process: " << e.what() << std::endl;
    }
}


int main(int argc, char* argv[]) {
    if (argc > 1 && std::string(argv[1]) == "cook") {
        run_cook(argc, argv);
    } else if (argc == 2) {
        run_file(argv[1]);
    } else if (argc == 1) {
        run_prompt();
    }
     else {
        std::cout << "Usage: yini [script|cook ...]" << std::endl;
        return 64; // EX_USAGE
    }

    return 0;
}