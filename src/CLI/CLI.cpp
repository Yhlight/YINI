#include "CLI.h"
#include "../Parser/YiniLoader.h"
#include "../Parser/Ymeta.h"
#include <iostream>
#include <sstream>
#include <vector>

namespace
{
    void printValue(const YiniValue& value, int indent = 0);
    void printIndent(int indent)
    {
        for (int i = 0; i < indent; ++i)
        {
            std::cout << "  ";
        }
    }
    void printArray(const YiniArray& arr, int indent)
    {
        std::cout << "[\n";
        for (const auto& val : arr)
        {
            printIndent(indent + 1);
            printValue(val, indent + 1);
            std::cout << ",\n";
        }
        printIndent(indent);
        std::cout << "]";
    }
    void printCoord(const YiniCoord& coord)
    {
        std::cout << "Coord(" << coord.x << ", " << coord.y << (coord.is_3d ? ", " + std::to_string(coord.z) : "") << ")";
    }
    void printValue(const YiniValue& yiniValue, int indent)
    {
        std::visit([&](auto&& arg)
        {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::string>) std::cout << '"' << arg << '"';
            else if constexpr (std::is_same_v<T, int64_t>) std::cout << arg;
            else if constexpr (std::is_same_v<T, double>) std::cout << arg;
            else if constexpr (std::is_same_v<T, bool>) std::cout << (arg ? "true" : "false");
            else if constexpr (std::is_same_v<T, YiniArray>) printArray(arg, indent);
            else if constexpr (std::is_same_v<T, YiniCoord>) printCoord(arg);
            else if constexpr (std::is_same_v<T, YiniMacroRef>) std::cout << "@" << arg.name;
            else std::cout << "Unhandled_Type";
        }, yiniValue.value);
    }
    void printAst(const YiniFile& ast)
    {
        if (!ast.definesMap.empty())
        {
            std::cout << "[#define]\n";
            for (const auto& [key, val] : ast.definesMap)
            {
                std::cout << "  " << key << " = ";
                printValue(val);
                std::cout << "\n";
            }
            std::cout << "\n";
        }
        if (!ast.includePaths.empty())
        {
            std::cout << "[#include]\n";
            for (const auto& path : ast.includePaths)
            {
                std::cout << "  += \"" << path << "\"\n";
            }
            std::cout << "\n";
        }
        for (const auto& [name, section] : ast.sectionsMap)
        {
            std::cout << "[" << name << "]";
            if (!section.inherits.empty())
            {
                std::cout << " : ";
                for (size_t i = 0; i < section.inherits.size(); ++i)
                {
                    std::cout << section.inherits[i] << (i < section.inherits.size() - 1 ? ", " : "");
                }
            }
            std::cout << "\n";
            for (const auto& [key, val] : section.keyValues)
            {
                std::cout << "  " << key << " = ";
                printValue(val, 1);
                std::cout << "\n";
            }
            for (const auto& val : section.autoIndexedValues)
            {
                std::cout << "  += ";
                printValue(val, 1);
                std::cout << "\n";
            }
            std::cout << "\n";
        }
    }
}

void CLI::run()
{
    printWelcomeMessage();
    mainLoop();
}

void CLI::printWelcomeMessage()
{
    std::cout << "YINI Language CLI Tool" << std::endl;
    std::cout << "Type 'help' for commands or 'exit' to quit." << std::endl;
}

void CLI::mainLoop()
{
    std::string line;
    while (true)
    {
        std::cout << "yini> ";
        if (!std::getline(std::cin, line) || line == "exit" || line == "quit")
        {
            break;
        }
        if (line.empty())
        {
            continue;
        }
        processInput(line);
    }
}

void CLI::processInput(const std::string& line)
{
    std::stringstream ss(line);
    std::string command;
    ss >> command;

    std::vector<std::string> args;
    std::string arg;
    while (ss >> arg)
    {
        args.push_back(arg);
    }

    if (command == "compile") cmdCompile(args);
    else if (command == "decompile") cmdDecompile(args);
    else if (command == "check") cmdCheck(args);
    else if (command == "help") cmdHelp();
    else std::cout << "Unknown command: '" << command << "'. Type 'help' for a list of commands." << std::endl;
}

void CLI::cmdCompile(const std::vector<std::string>& args)
{
    if (args.empty())
    {
        std::cout << "Usage: compile <input_file.yini>" << std::endl;
        return;
    }
    const std::string& inputFile = args[0];
    if (inputFile.size() < 6 || inputFile.substr(inputFile.size() - 5) != ".yini")
    {
        std::cout << "Error: Input file must have a .yini extension." << std::endl;
        return;
    }

    std::cout << "Compiling " << inputFile << "..." << std::endl;
    try
    {
        Yini::Loader loader;
        YiniFile ast = loader.load(inputFile);

        std::string outputFile = inputFile.substr(0, inputFile.size() - 5) + ".ymeta";
        Ymeta::Serializer serializer;
        serializer.serialize(ast, outputFile);

        std::cout << "Success! Compiled to " << outputFile << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Compilation failed: " << e.what() << std::endl;
    }
}

void CLI::cmdDecompile(const std::vector<std::string>& args)
{
    if (args.empty())
    {
        std::cout << "Usage: decompile <input_file.ymeta>" << std::endl;
        return;
    }
    const std::string& inputFile = args[0];

    std::cout << "Decompiling " << inputFile << "..." << std::endl;
    try
    {
        Ymeta::Deserializer deserializer;
        YiniFile ast = deserializer.deserialize(inputFile);
        printAst(ast);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Decompilation failed: " << e.what() << std::endl;
    }
}

void CLI::cmdCheck(const std::vector<std::string>& args)
{
    if (args.empty())
    {
        std::cout << "Usage: check <input_file.yini>" << std::endl;
        return;
    }
    const std::string& inputFile = args[0];

    std::cout << "Checking " << inputFile << "..." << std::endl;
    try
    {
        Yini::Loader loader;
        loader.load(inputFile);
        std::cout << "Syntax OK." << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Check failed: " << e.what() << std::endl;
    }
}

void CLI::cmdHelp()
{
    std::cout << "YINI CLI Commands:\n"
              << "  compile <file.yini>    - Compiles a .yini file and its includes into a binary .ymeta file.\n"
              << "  decompile <file.ymeta> - Reads a .ymeta file and prints its contents in a human-readable format.\n"
              << "  check <file.yini>      - Checks the syntax of a .yini file and its dependencies without generating output.\n"
              << "  help                   - Displays this help message.\n"
              << "  exit / quit            - Exits the CLI." << std::endl;
}
