#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "Runtime/Runtime.h"

// Helper to read a whole file into a string
std::string readFile(const std::string& filepath)
{
    std::ifstream file(filepath);
    if (!file)
    {
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void command_check(const std::string& filepath)
{
    if (filepath.empty())
    {
        std::cout << "Usage: check <filepath>" << std::endl;
        return;
    }

    std::cout << "Checking file: " << filepath << std::endl;
    std::string content = readFile(filepath);
    if (content.empty())
    {
        std::cout << "Error: Could not read file or file is empty." << std::endl;
        return;
    }

    Yini::Lexer lexer(content);
    Yini::Parser parser(lexer);
    auto doc = parser.parseDocument();

    const auto& errors = parser.getErrors();
    if (errors.empty())
    {
        std::cout << "Syntax OK." << std::endl;
    }
    else
    {
        std::cout << "Found " << errors.size() << " syntax errors:" << std::endl;
        for (const auto& err : errors)
        {
            std::cout << "  - " << err << std::endl;
        }
    }
}

void command_compile(const std::string& infile, const std::string& outfile)
{
    if (infile.empty() || outfile.empty()) {
        std::cout << "Usage: compile <infile.yini> <outfile.ymeta>" << std::endl;
        return;
    }
    std::cout << "Compiling " << infile << " to " << outfile << std::endl;

    std::string content = readFile(infile);
    if (content.empty()) {
        std::cout << "Error: Could not read file or file is empty." << std::endl;
        return;
    }

    Yini::Lexer lexer(content);
    Yini::Parser parser(lexer);
    auto doc = parser.parseDocument();
    if (!parser.getErrors().empty()) {
        std::cout << "Cannot compile due to syntax errors." << std::endl;
        return;
    }

    Yini::YiniRuntime runtime;
    runtime.evaluate(doc.get());

    if (runtime.serialize(outfile)) {
        std::cout << "Compilation successful." << std::endl;
    } else {
        std::cout << "Error during compilation/serialization." << std::endl;
    }
}

void command_decompile(const std::string& infile, const std::string& outfile)
{
    if (infile.empty() || outfile.empty()) {
        std::cout << "Usage: decompile <infile.ymeta> <outfile.yini>" << std::endl;
        return;
    }
    std::cout << "Decompiling " << infile << " to " << outfile << std::endl;

    Yini::YiniRuntime runtime;
    if (!runtime.deserialize(infile)) {
        std::cout << "Error: Failed to read or parse .ymeta file." << std::endl;
        return;
    }

    std::ofstream os(outfile);
    if (!os) {
        std::cout << "Error: Could not open output file " << outfile << std::endl;
        return;
    }

    os << runtime.dump();
    std::cout << "Decompilation successful." << std::endl;
}

void print_help()
{
    std::cout << "YINI Command-Line Interface" << std::endl;
    std::cout << "Available commands:" << std::endl;
    std::cout << "  check <filepath>        - Check the syntax of a .yini file." << std::endl;
    std::cout << "  compile <in> <out>      - Compile a .yini file to .ymeta." << std::endl;
    std::cout << "  decompile <in> <out>    - Decompile a .ymeta file to .yini." << std::endl;
    std::cout << "  help                    - Show this help message." << std::endl;
    std::cout << "  exit                    - Exit the application." << std::endl;
}

int main()
{
    std::string line;
    print_help();

    while (true)
    {
        std::cout << "> ";
        if (!std::getline(std::cin, line))
        {
            break;
        }

        std::stringstream ss(line);
        std::string command;
        ss >> command;

        if (command == "exit")
        {
            break;
        }
        else if (command == "help")
        {
            print_help();
        }
        else if (command == "check")
        {
            std::string filepath;
            ss >> filepath;
            command_check(filepath);
        }
        else if (command == "compile")
        {
            std::string infile, outfile;
            ss >> infile >> outfile;
            command_compile(infile, outfile);
        }
        else if (command == "decompile")
        {
            std::string infile, outfile;
            ss >> infile >> outfile;
            command_decompile(infile, outfile);
        }
        else if (!command.empty())
        {
            std::cout << "Unknown command: " << command << ". Type 'help' for a list of commands." << std::endl;
        }
    }

    std::cout << "Exiting YINI CLI." << std::endl;
    return 0;
}
