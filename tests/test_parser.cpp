#include "../src/Lexer/Lexer.h"
#include "../src/Parser/Parser.h"
#include <iostream>

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
    // Add other types later
}


int main() {
    std::string source = R"yini(
[#define]
name = "YINI"
version = 1

[#include]
+= "common.yini"

[TestSection] : BaseSection
key1 = 123
key2 = -45.6
is_true = true
name_ref = @name
+= "value1"
arr = [1, "two", 3.0]
)yini";

    YINI::Lexer lexer(source);
    auto tokens = lexer.tokenize();

    YINI::Parser parser(tokens);
    try {
        YINI::Document doc = parser.parse();

        std::cout << "--- Defines ---" << std::endl;
        for (const auto& def : doc.defines) {
            std::cout << def.first << " = ";
            printValue(def.second);
            std::cout << std::endl;
        }

        std::cout << "\n--- Includes ---" << std::endl;
        for (const auto& inc : doc.includes) {
            std::cout << "+= \"" << inc << "\"" << std::endl;
        }

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
            for (const auto& val : sec.anonymous_values) {
                std::cout << "+= ";
                printValue(val);
                std::cout << std::endl;
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "Parser error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
