#include "Core/YiniManager.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Deserializer.h"
#include "Core/YiniException.h"
#include "Core/YiniValue.h"
#include "Core/DynaValue.h"
#include "Core/Validator.h"
#include <iostream>
#include <string>
#include <vector>
#include <variant>

void print_usage() {
    std::cerr << "Usage: yini-cli <command> [args...]\n"
              << "Commands:\n"
              << "  check <filepath>        Check the syntax of a .yini file.\n"
              << "  validate <filepath>     Validate a .yini file against its embedded schema.\n"
              << "  compile <in> <out>      Compile a .yini file to .ymeta.\n"
              << "  decompile <filepath>    Decompile and print a .ymeta file.\n";
}

// Forward declarations for recursive printing
void print_value(const YINI::YiniValue& value, int indent);
void print_map(const YINI::YiniMap& map, int indent);

// Visitor for printing a YiniValue
struct PrintVisitor {
    int indent;

    void operator()(std::monostate) const { std::cout << "nil"; }
    void operator()(bool value) const { std::cout << (value ? "true" : "false"); }
    void operator()(double value) const { std::cout << value; }
    void operator()(const std::string& value) const { std::cout << "\"" << value << "\""; }

    void operator()(const std::unique_ptr<YINI::YiniArray>& value) const {
        std::cout << "[ ";
        for (const auto& item : *value) {
            print_value(item, indent);
            std::cout << " ";
        }
        std::cout << "]";
    }

    void operator()(const std::unique_ptr<YINI::YiniMap>& value) const {
        std::cout << "{\n";
        print_map(*value, indent + 2);
        std::cout << std::string(indent, ' ') << "}";
    }

    void operator()(const std::unique_ptr<YINI::DynaValue>& value) const {
        std::cout << "Dyna(";
        if (value && value->m_value) {
            print_value(*(value->m_value), indent);
        } else {
            std::cout << "nil";
        }
        std::cout << ")";
    }
};

void print_value(const YINI::YiniValue& value, int indent = 0) {
    std::visit(PrintVisitor{indent}, value.m_value);
}

void print_map(const YINI::YiniMap& map, int indent = 0) {
    for (const auto& pair : map) {
        std::cout << std::string(indent, ' ') << pair.first << ": ";
        print_value(pair.second, indent);
        std::cout << "\n";
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
            std::cout << "File '" << argv[2] << "' is syntactically valid." << std::endl;
        } else if (command == "validate" && argc == 3) {
            YINI::YiniManager manager;
            manager.load(argv[2]);
            const YINI::Schema* schema = manager.get_schema();
            if (!schema) {
                std::cerr << "Error: No [#schema] block found in '" << argv[2] << "'." << std::endl;
                return 1;
            }
            YINI::Validator validator;
            std::vector<YINI::ValidationError> errors = validator.validate(*schema, manager.get_interpreter());
            if (errors.empty()) {
                std::cout << "File '" << argv[2] << "' successfully validated against its schema." << std::endl;
            } else {
                std::cerr << "Validation failed with " << errors.size() << " error(s):" << std::endl;
                for (const auto& error : errors) {
                    std::cerr << "- " << error.message << std::endl;
                }
                return 1;
            }
        } else if (command == "compile" && argc == 4) {
            YINI::YiniManager manager;
            manager.load(argv[2]);
            YINI::Serialization::Serializer serializer;
            serializer.serialize(manager.get_interpreter().resolved_sections, argv[3]);
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
    } catch (const YINI::YiniException& e) {
        if (!e.filepath().empty()) {
            std::cerr << "[" << e.filepath() << ":" << e.line() << ":" << e.column() << "] Error: " << e.what() << std::endl;
        } else {
            std::cerr << "[line " << e.line() << "] Error: " << e.what() << std::endl;
        }
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}