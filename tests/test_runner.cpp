#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <cassert>
#include <sstream>

#include "../src/Lexer/Lexer.h"
#include "../src/Parser/Parser.h"
#include "../src/Processor/Processor.h"
#include "../src/YMeta/Serializer.h"
#include "../src/YMeta/Deserializer.h"

// Simple Test Harness
int total_tests = 0;
int failed_tests = 0;
std::vector<std::string> failures;

// A simple assertion macro
#define ASSERT(condition) \
    if (!(condition)) { \
        throw std::runtime_error("Assertion failed: " #condition); \
    }

void test(const std::string& test_name, std::function<void()> test_func)
{
    total_tests++;
    try {
        test_func();
        std::cout << "[ \033[32mPASS\033[0m ] " << test_name << std::endl;
    } catch (const std::exception& e) {
        failed_tests++;
        std::cout << "[ \033[31mFAIL\033[0m ] " << test_name << std::endl;
        failures.push_back("       " + test_name + ": " + e.what());
    } catch (...) {
        failed_tests++;
        std::cout << "[ \033[31mFAIL\033[0m ] " << test_name << std::endl;
        failures.push_back("       " + test_name + ": Unknown exception");
    }
}

// Helper function to compare two YiniFile ASTs
bool compareASTs(const YINI::YiniFile& a, const YINI::YiniFile& b) {
    if (a.sections.size() != b.sections.size()) return false;
    for (auto const& [key, val_a] : a.sections) {
        auto it = b.sections.find(key);
        if (it == b.sections.end()) return false;
        const auto& val_b = it->second;
        if (val_a.name != val_b.name) return false;
        if (val_a.pairs.size() != val_b.pairs.size()) return false;
        for (size_t i = 0; i < val_a.pairs.size(); ++i) {
            if (val_a.pairs[i].key != val_b.pairs[i].key) return false;
            if (!(*val_a.pairs[i].value == *val_b.pairs[i].value)) return false;
        }
    }
    return true;
}


// --- Test Suite Implementations ---
void run_lexer_tests()
{
    test("Lexer: All Tokens", []() {
        std::string input = "[S]:P+=1 1.2 t #123456 (1,2){k:v}@m";
        YINI::Lexer l(input);
        ASSERT(l.getNextToken().type == YINI::TokenType::Section);
        ASSERT(l.getNextToken().type == YINI::TokenType::Colon);
        ASSERT(l.getNextToken().type == YINI::TokenType::Identifier);
        ASSERT(l.getNextToken().type == YINI::TokenType::PlusAssign);
        ASSERT(l.getNextToken().type == YINI::TokenType::Integer);
        ASSERT(l.getNextToken().type == YINI::TokenType::Float);
        ASSERT(l.getNextToken().type == YINI::TokenType::Identifier); // 't' is just an identifier
        ASSERT(l.getNextToken().type == YINI::TokenType::Color);
        ASSERT(l.getNextToken().type == YINI::TokenType::LeftParen);
        ASSERT(l.getNextToken().type == YINI::TokenType::Integer);
        ASSERT(l.getNextToken().type == YINI::TokenType::Comma);
        ASSERT(l.getNextToken().type == YINI::TokenType::Integer);
        ASSERT(l.getNextToken().type == YINI::TokenType::RightParen);
        ASSERT(l.getNextToken().type == YINI::TokenType::LeftBrace);
        ASSERT(l.getNextToken().type == YINI::TokenType::Identifier);
        ASSERT(l.getNextToken().type == YINI::TokenType::Colon);
        ASSERT(l.getNextToken().type == YINI::TokenType::Identifier);
        ASSERT(l.getNextToken().type == YINI::TokenType::RightBrace);
        ASSERT(l.getNextToken().type == YINI::TokenType::Macro);
    });
}

void run_parser_tests()
{
    test("Parser: All Value Types", []() {
        std::string input = "[Test]\n"
                            "arr = [1, [2, 3]]\n"
                            "map = {\"k1\": 1, \"k2\": true}\n"
                            "col = Color(1,2,3)";
        YINI::Lexer lexer(input);
        YINI::Parser parser(lexer);
        auto ast = parser.parse();
        ASSERT(ast->sections.size() == 1);
        const auto& section = ast->sections.at("Test");
        ASSERT(section.pairs.size() == 3);
        ASSERT(std::holds_alternative<YINI::Array>(section.pairs[0].value->data));
        ASSERT(std::holds_alternative<YINI::Map>(section.pairs[1].value->data));
        ASSERT(std::holds_alternative<YINI::Color>(section.pairs[2].value->data));
    });

    test("Parser: Syntax Error", []() {
        bool thrown = false;
        try {
            std::string input = "[Test]\nkey =";
            YINI::Lexer lexer(input);
            YINI::Parser parser(lexer);
            parser.parse();
        } catch (const std::exception&) {
            thrown = true;
        }
        ASSERT(thrown);
    });
}

void run_processor_tests()
{
    test("Processor: Inheritance and Macros", []() {
        std::string input = "[#define]\nname=\"Test\"\n[Base]\na=1\n[Child]:Base\nname=@name";
        YINI::Lexer lexer(input);
        YINI::Parser parser(lexer);
        auto ast = parser.parse();
        YINI::Processor processor(std::move(ast));
        auto processed_ast = processor.process();
        const auto& section = processed_ast->sections.at("Child");
        ASSERT(section.pairs.size() == 2); // a=1, name="Test"
        bool name_found = false;
        for(const auto& p : section.pairs) {
            if(p.key == "name" && std::get<YINI::String>(p.value->data) == "Test") {
                name_found = true;
            }
        }
        ASSERT(name_found);
    });
}

void run_ymeta_tests()
{
    test("YMeta: Round-trip with deep comparison", []() {
        std::string input = "[Section]\nkey = [1,2,3]";
        YINI::Lexer lexer(input);
        YINI::Parser parser(lexer);
        auto ast1 = parser.parse();
        YINI::Processor processor(std::move(ast1));
        auto processed_ast = processor.process();

        std::stringstream buffer;
        YINI::Serializer serializer(*processed_ast);
        serializer.serialize(buffer);

        YINI::Deserializer deserializer(buffer);
        auto deserialized_ast = deserializer.deserialize();

        ASSERT(compareASTs(*processed_ast, *deserialized_ast));
    });
}

int main()
{
    std::cout << "--- Running YINI Test Suite ---" << std::endl;
    run_lexer_tests();
    run_parser_tests();
    run_processor_tests();
    run_ymeta_tests();
    std::cout << "\n--- Test Summary ---" << std::endl;
    if (failed_tests == 0) {
        std::cout << "\033[32mAll " << total_tests << " tests passed!\033[0m" << std::endl;
        return 0;
    } else {
        std::cout << "\033[31m" << failed_tests << " out of " << total_tests << " tests failed.\033[0m" << std::endl;
        for (const auto& failure : failures) {
            std::cout << failure << std::endl;
        }
        return 1;
    }
}
