#include <iostream>
#include <fstream>
#include <streambuf>
#include <string>
#include <vector>
#include <sstream>
#include "YINI/YiniManager.hpp"
#include "YINI/YiniException.hpp"
#include "YINI/Parser.hpp"
#include "YINI/JsonDeserializer.hpp"

void printHelp() {
    std::cout << "YINI CLI - Interactive Mode\n";
    std::cout << "Available commands:\n";
    std::cout << "  check <filepath>     - Checks the syntax of a .yini file.\n";
    std::cout << "  compile <filepath>   - Compiles a .yini file to .ymeta.\n";
    std::cout << "  decompile <filepath> - Decompiles a .ymeta file to a readable format.\n";
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
    } catch (const YINI::YiniException& e) {
        std::cerr << "Syntax Error in " << filePath << " [" << e.getLine() << ":" << e.getColumn() << "]: " << e.what() << std::endl;
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
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file: " << filePath << std::endl;
        return;
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    YINI::YiniDocument doc;
    if (YINI::JsonDeserializer::deserialize(content, doc)) {
        std::cout << "--- Decompilation of " << filePath << " ---\n\n";

        const auto& defines = doc.getDefines();
        if (!defines.empty()) {
            std::cout << "[#define]\n";
            for (const auto& define_pair : defines) {
                // For simplicity, we don't display the full value structure here.
                std::cout << "  " << define_pair.first << " = ...\n";
            }
            std::cout << "\n";
        }

        for (const auto& section : doc.getSections()) {
            std::cout << "[" << section.name << "]";
            if (!section.inheritedSections.empty()) {
                std::cout << " : ";
                for (size_t i = 0; i < section.inheritedSections.size(); ++i) {
                    std::cout << section.inheritedSections[i] << (i < section.inheritedSections.size() - 1 ? ", " : "");
                }
            }
            std::cout << "\n";

            for (const auto& pair : section.pairs) {
                std::cout << "  " << pair.key << " = ...\n";
            }
            for (const auto& val : section.registrationList) {
                std::cout << "  += ...\n";
            }
            std::cout << "\n";
        }
        std::cout << "--- End of Decompilation ---\n";
    } else {
        std::cerr << "Error: Failed to decompile " << filePath << ". It may be corrupted or not a valid .ymeta file." << std::endl;
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