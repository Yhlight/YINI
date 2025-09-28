#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "Lexer.h"
#include "Parser.h"

void run_file(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << "Could not open file: " << path << std::endl;
        return;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    YINI::Lexer lexer(source);
    std::vector<YINI::Token> tokens = lexer.scanTokens();

    YINI::Parser parser(tokens);
    parser.parse();

    if (parser.hasError())
    {
        exit(65); // EX_DATAERR
    }
}

int main(int argc, char* argv[])
{
    if (argc > 2)
    {
        std::cout << "Usage: yini-cli [script]" << std::endl;
        return 64; // EX_USAGE
    }
    else if (argc == 2)
    {
        run_file(argv[1]);
    }
    else
    {
        // No file provided, interactive mode could be implemented here
        std::cout << "YINI CLI (no file specified)" << std::endl;
    }

    return 0;
}