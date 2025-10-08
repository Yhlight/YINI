#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "Loader/Loader.h"
#include "Parser/Ast.h"


void printValue(const Yini::Value& value);

void printArray(const Yini::ArrayValue& array) {
    std::cout << "[";
    for (size_t i = 0; i < array.elements.size(); ++i) {
        printValue(*array.elements[i]);
        if (i < array.elements.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]";
}

void printValue(const Yini::Value& value) {
    switch (value.getType()) {
        case Yini::ValueType::Identifier:
            std::cout << dynamic_cast<const Yini::IdentifierValue&>(value).token.lexeme;
            break;
        case Yini::ValueType::String:
            std::cout << '"' << dynamic_cast<const Yini::StringValue&>(value).value << '"';
            break;
        case Yini::ValueType::Number:
            std::cout << dynamic_cast<const Yini::NumberValue&>(value).value;
            break;
        case Yini::ValueType::Bool:
            std::cout << (dynamic_cast<const Yini::BoolValue&>(value).value ? "true" : "false");
            break;
        case Yini::ValueType::Array:
             printArray(dynamic_cast<const Yini::ArrayValue&>(value));
            break;
        // Other types can be added here
        default:
            std::cout << "UNPRINTABLE_VALUE";
            break;
    }
}

void printAst(const std::vector<std::unique_ptr<Yini::SectionNode>>& ast) {
    for (const auto& section : ast) {
        std::cout << "[" << section->name.lexeme << "]" << std::endl;
        for (const auto& pair : section->pairs) {
            std::cout << pair->key.lexeme << " = ";
            printValue(*pair->value);
            std::cout << std::endl;
        }
    }
}

std::string changeExtension(const std::string& filename, const std::string& newExtension) {
    size_t lastdot = filename.find_last_of(".");
    if (lastdot == std::string::npos) return filename + newExtension;
    return filename.substr(0, lastdot) + newExtension;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: yini_cli <command> <file>" << std::endl;
        return 1;
    }

    std::string command = argv[1];
    std::string filename = argv[2];
    Yini::Loader loader;

    if (command == "compile") {
        try {
            auto ast = loader.load(filename);
            std::string outFilename = changeExtension(filename, ".ymeta");
            std::ofstream outFile(outFilename, std::ios::binary);
            if (!outFile.is_open()) {
                std::cerr << "Error: Could not open output file " << outFilename << std::endl;
                return 1;
            }

            size_t sectionCount = ast.size();
            outFile.write(reinterpret_cast<const char*>(&sectionCount), sizeof(sectionCount));

            for(const auto& section : ast) {
                section->serialize(outFile);
            }
            std::cout << "File compiled successfully to " << outFilename << std::endl;

        } catch (const std::exception& e) {
            std::cerr << "Error compiling file: " << e.what() << std::endl;
            return 1;
        }
    } else if (command == "decompile") {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filename << std::endl;
            return 1;
        }

        try {
            size_t sectionCount;
            file.read(reinterpret_cast<char*>(&sectionCount), sizeof(sectionCount));

            std::vector<std::unique_ptr<Yini::SectionNode>> ast;
            for(size_t i = 0; i < sectionCount; ++i) {
                ast.push_back(Yini::deserializeSection(file));
            }

            printAst(ast);

        } catch (const std::exception& e) {
            std::cerr << "Error decompiling file: " << e.what() << std::endl;
            return 1;
        }

    } else if (command == "parse") {
        try {
            auto ast = loader.load(filename);
            std::cout << "File parsed successfully." << std::endl;
            printAst(ast);
        } catch (const std::exception& e) {
            std::cerr << "Error parsing file: " << e.what() << std::endl;
            return 1;
        }
    } else if (command == "check") {
        try {
            auto ast = loader.load(filename);
            std::cout << "File syntax is valid." << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "File syntax is invalid: " << e.what() << std::endl;
            return 1;
        }
    }
    else {
        std::cerr << "Unknown command: " << command << std::endl;
        return 1;
    }

    return 0;
}