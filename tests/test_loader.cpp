#include "../src/Loader.h"
#include <iostream>

// Using the same print helpers from test_parser.cpp
// In a real project, these would be in a shared utility file.
void printValue(const YINI::Value& value);

void printArray(const std::unique_ptr<YINI::Array>& array) {
    std::cout << "[";
    for (size_t i = 0; i < array->elements.size(); ++i) {
        printValue(array->elements[i]);
        if (i < array->elements.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]";
}

void printValue(const YINI::Value& value) {
    if (std::holds_alternative<std::string>(value)) {
        std::cout << "\"" << std::get<std::string>(value) << "\"";
    } else if (std::holds_alternative<long long>(value)) {
        std::cout << std::get<long long>(value);
    } else if (std::holds_alternative<double>(value)) {
        std::cout << std::get<double>(value);
    } else if (std::holds_alternative<bool>(value)) {
        std::cout << (std::get<bool>(value) ? "true" : "false");
    } else if (std::holds_alternative<std::unique_ptr<YINI::Array>>(value)) {
        printArray(std::get<std::unique_ptr<YINI::Array>>(value));
    }
}


int main() {
    YINI::Loader loader;
    try {
        YINI::Document doc = loader.load("tests/include_main.yini");

        std::cout << "--- Final Merged Document ---" << std::endl;

        std::cout << "\n--- Sections ---" << std::endl;
        for (const auto& sec : doc.sections) {
            std::cout << "[" << sec.name << "]";
            if (!sec.inherited_sections.empty()) {
                std::cout << " : ";
                for (size_t i = 0; i < sec.inherited_sections.size(); ++i) {
                    std::cout << sec.inherited_sections[i] << (i < sec.inherited_sections.size() - 1 ? ", " : "");
                }
            }
            std::cout << std::endl;

            for (const auto& pair : sec.pairs) {
                std::cout << pair.key << " = ";
                printValue(pair.value);
                std::cout << std::endl;
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "Loader error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
