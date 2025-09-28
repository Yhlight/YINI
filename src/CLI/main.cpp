#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "Lexer.h"
#include "Parser.h"
#include "AstSerializer.h"

void check_file(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << "Could not open file: " << path << std::endl;
        exit(1);
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
        std::cerr << "Syntax errors found in " << path << std::endl;
        exit(65); // EX_DATAERR
    }
    else
    {
        std::cout << "Syntax OK: " << path << std::endl;
    }
}

void compile_file(const std::string& inputPath, const std::string& outputPath)
{
    std::ifstream file(inputPath);
    if (!file.is_open())
    {
        std::cerr << "Could not open input file: " << inputPath << std::endl;
        exit(1);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    YINI::Lexer lexer(source);
    std::vector<YINI::Token> tokens = lexer.scanTokens();

    YINI::Parser parser(tokens);
    auto ast = parser.parse();

    if (parser.hasError())
    {
        std::cerr << "Could not compile due to syntax errors: " << inputPath << std::endl;
        exit(65);
    }

    YINI::AstSerializer serializer;
    json ymeta = serializer.serialize(ast);

    std::ofstream outFile(outputPath);
    if (!outFile.is_open())
    {
        std::cerr << "Could not open output file: " << outputPath << std::endl;
        exit(1);
    }

    outFile << ymeta.dump(4);
    std::cout << "Successfully compiled " << inputPath << " to " << outputPath << std::endl;
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: yini-cli <command> [options]" << std::endl;
        std::cout << "Commands:" << std::endl;
        std::cout << "  check <file>      Check syntax of a .yini file" << std::endl;
        std::cout << "  compile <in> <out> Compile a .yini file to .ymeta" << std::endl;
        return 64; // EX_USAGE
    }

    std::string command = argv[1];
    if (command == "check" && argc == 3)
    {
        check_file(argv[2]);
    }
    else if (command == "compile" && argc == 4)
    {
        compile_file(argv[2], argv[3]);
    }
    else
    {
        std::cout << "Invalid command or arguments." << std::endl;
        return 64;
    }

    return 0;
}