#include "Core/YiniManager.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Deserializer.h"
#include "Core/YiniException.h"
#include <iostream>
#include <string>
#include <vector>
#include <any>

void print_usage() {
    std::cerr << "Usage: yini-cli <command> [args...]\n"
              << "Commands:\n"
              << "  check <filepath>        Validate a .yini file.\n"
              << "  compile <in> <out>      Compile a .yini file to .ymeta.\n"
              << "  decompile <filepath>    Decompile and print a .ymeta file.\n";
}

void print_any(const std::any& value);

void print_map(const std::map<std::string, std::any>& map, int indent = 0) {
    for (const auto& pair : map) {
        std::cout << std::string(indent, ' ') << pair.first << ": ";
        print_any(pair.second);
        std::cout << "\n";
    }
}

void print_any(const std::any& value) {
    if (value.type() == typeid(double)) {
        std::cout << std::any_cast<double>(value);
    } else if (value.type() == typeid(bool)) {
        std::cout << (std::any_cast<bool>(value) ? "true" : "false");
    } else if (value.type() == typeid(std::string)) {
        std::cout << "\"" << std::any_cast<std::string>(value) << "\"";
    } else if (value.type() == typeid(std::vector<std::any>)) {
        std::cout << "[ ";
        const auto& vec = std::any_cast<const std::vector<std::any>&>(value);
        for (const auto& item : vec) {
            print_any(item);
            std::cout << " ";
        }
        std::cout << "]";
    } else if (value.type() == typeid(std::map<std::string, std::any>)) {
        std::cout << "{\n";
        print_map(std::any_cast<const std::map<std::string, std::any>&>(value), 4);
        std::cout << std::string(2, ' ') << "}";
    } else {
        std::cout << "nil";
    }
}


int main(int argc, char* argv[])
{
    if (argc < 2) {
        print_usage();
        return 1;
    }

    std::string command = argv[1];

    try {
        if (command == "check" && argc == 3) {
            YINI::YiniManager manager;
            manager.load(argv[2]);
            std::cout << "File '" << argv[2] << "' is valid." << std::endl;
        } else if (command == "compile" && argc == 4) {
            YINI::YiniManager manager;
            manager.load(argv[2]);
            YINI::Serialization::Serializer serializer;
            serializer.serialize(manager.interpreter.resolved_sections, argv[3]);
            std::cout << "Compiled '" << argv[2] << "' to '" << argv[3] << "'." << std::endl;
        } else if (command == "decompile" && argc == 3) {
            YINI::Serialization::Deserializer deserializer;
            auto data = deserializer.deserialize(argv[2]);
            for (const auto& section : data) {
                std::cout << "[" << section.first << "]\n";
                print_map(section.second, 2);
            }
        } else {
            print_usage();
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}