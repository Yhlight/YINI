#include "../src/Lexer/Lexer.h"
#include "../src/Parser/Parser.h"
#include <iostream>
#include <cassert>

void test_empty_input() {
    std::cout << "Running test: empty input..." << std::endl;
    std::string source = "";
    YINI::Lexer lexer(source);
    auto tokens = lexer.tokenize();
    YINI::Parser parser(tokens);
    YINI::Document doc = parser.parse();
    assert(doc.defines.empty());
    assert(doc.includes.empty());
    assert(doc.sections.empty());
    std::cout << "PASSED" << std::endl;
}

void test_comments_only() {
    std::cout << "Running test: comments only..." << std::endl;
    std::string source = R"yini(
// this is a file with only comments
/* multi-line
   comment */
)yini";
    YINI::Lexer lexer(source);
    auto tokens = lexer.tokenize();
    YINI::Parser parser(tokens);
    YINI::Document doc = parser.parse();
    assert(doc.defines.empty());
    assert(doc.includes.empty());
    assert(doc.sections.empty());
    std::cout << "PASSED" << std::endl;
}


void test_malformed_section() {
    std::cout << "Running test: malformed section..." << std::endl;
    std::string source = "[Section";
    YINI::Lexer lexer(source);
    auto tokens = lexer.tokenize();
    YINI::Parser parser(tokens);
    bool exception_thrown = false;
    try {
        parser.parse();
    } catch (const std::runtime_error& e) {
        exception_thrown = true;
        std::string msg = e.what();
        assert(msg.find("Expect ']' after section header") != std::string::npos);
    }
    assert(exception_thrown);
    std::cout << "PASSED" << std::endl;
}

int main() {
    test_empty_input();
    test_comments_only();
    test_malformed_section();
    // Add more tests here...

    return 0;
}
