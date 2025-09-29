#include <iostream>
#include <fstream>
#include <streambuf>
#include <string>
#include <vector>
#include <sstream>
#include "YINI/YiniManager.hpp"
#include "YINI/YiniException.hpp"
#include "YINI/Parser.hpp"

#include "YINI/YiniFormatter.hpp"

void printHelp() {
    std::cout << "YINI CLI - Interactive Mode\n";
    std::cout << "Available commands:\n";
    std::cout << "  check <filepath>     - Checks the syntax of a .yini file.\n";
    std::cout << "  compile <filepath>   - Compiles a .yini file to .ymeta.\n";
    std::cout << "  decompile <filepath> - Decompiles a .ymeta file to standard output.\n";
    std::cout << "  help                 - Shows this help message.\n";
    std::cout << "  exit                 - Exits the CLI.\n";
}

void handleCheck(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file: " << filePath << std::endl;
        return;
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    try {
        YINI::YiniDocument doc;
        std::string basePath = ".";
        size_t last_slash_idx = filePath.rfind('/');
        if (std::string::npos != last_slash_idx)
        {
            basePath = filePath.substr(0, last_slash_idx);
        }
        YINI::Parser parser(content, doc, basePath);
        parser.parse();
        std::cout << "Syntax OK: " << filePath << std::endl;
    } catch (const YINI::YiniParsingException& e) {
        std::cerr << "Found " << e.getErrors().size() << " syntax errors in " << filePath << ":\n";
        for (const auto& err : e.getErrors()) {
            std::cerr << "  [" << err.line << ":" << err.column << "]: " << err.message << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
    }
}

void handleCompile(const std::string& filePath) {
    YINI::YiniManager manager(filePath);
    if (manager.isLoaded()) {
        std::cout << "Successfully compiled " << filePath << " to .ymeta" << std::endl;
    } else {
        std::cerr << "Error: Failed to load or compile " << filePath << ". Check for syntax errors or file access issues." << std::endl;
    }
}

void handleDecompile(const std::string& filePath) {
    // YiniManager will load from .ymeta if it exists, which is what we want.
    YINI::YiniManager manager(filePath);
    if (manager.isLoaded()) {
        std::string formatted_content = YINI::YiniFormatter::formatDocument(manager.getDocument());
        std::cout << formatted_content;
    } else {
        std::cerr << "Error: Could not load " << filePath << ". File may not exist or is corrupted." << std::endl;
    }
}

int main() {
    std::string line;
    printHelp();

    while (true) {
        std::cout << "> ";
        std::getline(std::cin, line);

        std::stringstream ss(line);
        std::string command;
        ss >> command;

        if (command == "exit") {
            break;
        } else if (command == "help") {
            printHelp();
        } else if (command == "check") {
            std::string filePath;
            ss >> filePath;
            if (filePath.empty()) {
                std::cerr << "Usage: check <filepath>" << std::endl;
            } else {
                handleCheck(filePath);
            }
        } else if (command == "compile") {
            std::string filePath;
            ss >> filePath;
            if (filePath.empty()) {
                std::cerr << "Usage: compile <filepath>" << std::endl;
            } else {
                handleCompile(filePath);
            }
        } else if (command == "decompile") {
            std::string filePath;
            ss >> filePath;
            if (filePath.empty()) {
                std::cerr << "Usage: decompile <filepath>" << std::endl;
            } else {
                handleDecompile(filePath);
            }
        } else if (!command.empty()) {
            std::cerr << "Unknown command: " << command << ". Type 'help' for a list of commands." << std::endl;
        }
    }

    return 0;
}