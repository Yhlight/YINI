#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"

static void run(const std::string& source) {
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    try {
        auto ast = parser.parse();
        // For now, we just check for parsing errors.
        // In the future, we would process the AST.
        std::cout << "Parsing completed successfully." << std::endl;
    } catch (const std::runtime_error& e) {
        std::cerr << "Parsing error: " << e.what() << std::endl;
    }
}

static void run_file(const char* path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file '" << path << "'" << std::endl;
        return;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    run(buffer.str());
}

static void run_prompt() {
    std::string line;
    for (;;) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) {
            std::cout << std::endl;
            break;
        }
        run(line);
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