#include "Lexer.h"
#include "Token.h"
#include <cassert>
#include <iostream>
#include <string>

using namespace yini;

void test_basic_tokens()
{
    std::cout << "Testing basic tokens..." << std::endl;
    
    Lexer lexer("[ ] ( ) { } , : = += + - * / %");
    auto tokens = lexer.tokenize();
    
    assert(tokens[0].type == TokenType::LBRACKET);
    assert(tokens[1].type == TokenType::RBRACKET);
    assert(tokens[2].type == TokenType::LPAREN);
    assert(tokens[3].type == TokenType::RPAREN);
    assert(tokens[4].type == TokenType::LBRACE);
    assert(tokens[5].type == TokenType::RBRACE);
    assert(tokens[6].type == TokenType::COMMA);
    assert(tokens[7].type == TokenType::COLON);
    assert(tokens[8].type == TokenType::EQUALS);
    assert(tokens[9].type == TokenType::PLUS_EQUALS);
    assert(tokens[10].type == TokenType::PLUS);
    assert(tokens[11].type == TokenType::MINUS);
    assert(tokens[12].type == TokenType::MULTIPLY);
    assert(tokens[13].type == TokenType::DIVIDE);
    assert(tokens[14].type == TokenType::MODULO);
    
    std::cout << "✓ Basic tokens test passed" << std::endl;
}

void test_integers()
{
    std::cout << "Testing integers..." << std::endl;
    
    Lexer lexer("123 456 0 999");
    auto tokens = lexer.tokenize();
    
    assert(tokens[0].type == TokenType::INTEGER);
    assert(tokens[0].getValue<int64_t>() == 123);
    
    assert(tokens[1].type == TokenType::INTEGER);
    assert(tokens[1].getValue<int64_t>() == 456);
    
    assert(tokens[2].type == TokenType::INTEGER);
    assert(tokens[2].getValue<int64_t>() == 0);
    
    std::cout << "✓ Integer test passed" << std::endl;
}

void test_floats()
{
    std::cout << "Testing floats..." << std::endl;
    
    Lexer lexer("3.14 2.5 0.1");
    auto tokens = lexer.tokenize();
    
    assert(tokens[0].type == TokenType::FLOAT);
    assert(tokens[0].getValue<double>() == 3.14);
    
    assert(tokens[1].type == TokenType::FLOAT);
    assert(tokens[1].getValue<double>() == 2.5);
    
    std::cout << "✓ Float test passed" << std::endl;
}

void test_booleans()
{
    std::cout << "Testing booleans..." << std::endl;
    
    Lexer lexer("true false");
    auto tokens = lexer.tokenize();
    
    assert(tokens[0].type == TokenType::BOOLEAN);
    assert(tokens[0].getValue<bool>() == true);
    
    assert(tokens[1].type == TokenType::BOOLEAN);
    assert(tokens[1].getValue<bool>() == false);
    
    std::cout << "✓ Boolean test passed" << std::endl;
}

void test_strings()
{
    std::cout << "Testing strings..." << std::endl;
    
    Lexer lexer(R"("hello" "world" "test\nvalue")");
    auto tokens = lexer.tokenize();
    
    assert(tokens[0].type == TokenType::STRING);
    assert(tokens[0].getValue<std::string>() == "hello");
    
    assert(tokens[1].type == TokenType::STRING);
    assert(tokens[1].getValue<std::string>() == "world");
    
    assert(tokens[2].type == TokenType::STRING);
    assert(tokens[2].getValue<std::string>() == "test\nvalue");
    
    std::cout << "✓ String test passed" << std::endl;
}

void test_identifiers()
{
    std::cout << "Testing identifiers..." << std::endl;
    
    Lexer lexer("key1 value name_test");
    auto tokens = lexer.tokenize();
    
    assert(tokens[0].type == TokenType::IDENTIFIER);
    assert(tokens[0].getValue<std::string>() == "key1");
    
    assert(tokens[1].type == TokenType::IDENTIFIER);
    assert(tokens[1].getValue<std::string>() == "value");
    
    std::cout << "✓ Identifier test passed" << std::endl;
}

void test_comments()
{
    std::cout << "Testing comments..." << std::endl;
    
    Lexer lexer("key1 // this is a comment\nkey2 /* block comment */ key3");
    auto tokens = lexer.tokenize();
    
    // Comments should be filtered out, but newlines are kept
    assert(tokens[0].type == TokenType::IDENTIFIER);
    assert(tokens[0].getValue<std::string>() == "key1");
    
    // tokens[1] is NEWLINE
    assert(tokens[1].type == TokenType::NEWLINE);
    
    assert(tokens[2].type == TokenType::IDENTIFIER);
    assert(tokens[2].getValue<std::string>() == "key2");
    
    assert(tokens[3].type == TokenType::IDENTIFIER);
    assert(tokens[3].getValue<std::string>() == "key3");
    
    std::cout << "✓ Comment test passed" << std::endl;
}

void test_builtin_types()
{
    std::cout << "Testing built-in types..." << std::endl;
    
    Lexer lexer("Color color Coord coord List list Array array Dyna dyna Path path");
    auto tokens = lexer.tokenize();
    
    assert(tokens[0].type == TokenType::COLOR);
    assert(tokens[1].type == TokenType::COLOR);
    assert(tokens[2].type == TokenType::COORD);
    assert(tokens[3].type == TokenType::COORD);
    assert(tokens[4].type == TokenType::LIST);
    assert(tokens[5].type == TokenType::LIST);
    assert(tokens[6].type == TokenType::ARRAY);
    assert(tokens[7].type == TokenType::ARRAY);
    assert(tokens[8].type == TokenType::DYNA);
    assert(tokens[9].type == TokenType::DYNA);
    
    std::cout << "✓ Built-in types test passed" << std::endl;
}

void test_color_hex()
{
    std::cout << "Testing hex colors..." << std::endl;
    
    Lexer lexer("#FF0000 #00FF00 #0000FF");
    auto tokens = lexer.tokenize();
    
    assert(tokens[0].type == TokenType::COLOR);
    assert(tokens[0].getValue<std::string>() == "#FF0000");
    
    assert(tokens[1].type == TokenType::COLOR);
    assert(tokens[1].getValue<std::string>() == "#00FF00");
    
    std::cout << "✓ Hex color test passed" << std::endl;
}

void test_special_symbols()
{
    std::cout << "Testing special symbols..." << std::endl;
    
    Lexer lexer("@ @{ ${ # ! ? ~");
    auto tokens = lexer.tokenize();
    
    assert(tokens[0].type == TokenType::AT);
    assert(tokens[1].type == TokenType::AT_LBRACE);
    assert(tokens[2].type == TokenType::DOLLAR_LBRACE);
    assert(tokens[3].type == TokenType::HASH);
    assert(tokens[4].type == TokenType::EXCLAMATION);
    assert(tokens[5].type == TokenType::QUESTION);
    assert(tokens[6].type == TokenType::TILDE);
    
    std::cout << "✓ Special symbols test passed" << std::endl;
}

void test_section_header()
{
    std::cout << "Testing section header..." << std::endl;
    
    Lexer lexer("[Config]");
    auto tokens = lexer.tokenize();
    
    assert(tokens[0].type == TokenType::LBRACKET);
    assert(tokens[1].type == TokenType::IDENTIFIER);
    assert(tokens[1].getValue<std::string>() == "Config");
    assert(tokens[2].type == TokenType::RBRACKET);
    
    std::cout << "✓ Section header test passed" << std::endl;
}

void test_key_value_pair()
{
    std::cout << "Testing key-value pair..." << std::endl;
    
    Lexer lexer("key = value");
    auto tokens = lexer.tokenize();
    
    assert(tokens[0].type == TokenType::IDENTIFIER);
    assert(tokens[0].getValue<std::string>() == "key");
    assert(tokens[1].type == TokenType::EQUALS);
    assert(tokens[2].type == TokenType::IDENTIFIER);
    assert(tokens[2].getValue<std::string>() == "value");
    
    std::cout << "✓ Key-value pair test passed" << std::endl;
}

void test_array_syntax()
{
    std::cout << "Testing array syntax..." << std::endl;
    
    Lexer lexer("[1, 2, 3]");
    auto tokens = lexer.tokenize();
    
    assert(tokens[0].type == TokenType::LBRACKET);
    assert(tokens[1].type == TokenType::INTEGER);
    assert(tokens[2].type == TokenType::COMMA);
    assert(tokens[3].type == TokenType::INTEGER);
    assert(tokens[4].type == TokenType::COMMA);
    assert(tokens[5].type == TokenType::INTEGER);
    assert(tokens[6].type == TokenType::RBRACKET);
    
    std::cout << "✓ Array syntax test passed" << std::endl;
}

void test_inheritance_syntax()
{
    std::cout << "Testing inheritance syntax..." << std::endl;
    
    Lexer lexer("[Config3] : Config, Config2");
    auto tokens = lexer.tokenize();
    
    assert(tokens[0].type == TokenType::LBRACKET);
    assert(tokens[1].type == TokenType::IDENTIFIER);
    assert(tokens[1].getValue<std::string>() == "Config3");
    assert(tokens[2].type == TokenType::RBRACKET);
    assert(tokens[3].type == TokenType::COLON);
    assert(tokens[4].type == TokenType::IDENTIFIER);
    assert(tokens[4].getValue<std::string>() == "Config");
    
    std::cout << "✓ Inheritance syntax test passed" << std::endl;
}

void test_arithmetic_expression()
{
    std::cout << "Testing arithmetic expression..." << std::endl;
    
    Lexer lexer("1 + 2 * 3 - 4 / 5 % 6");
    auto tokens = lexer.tokenize();
    
    assert(tokens[0].type == TokenType::INTEGER);
    assert(tokens[1].type == TokenType::PLUS);
    assert(tokens[2].type == TokenType::INTEGER);
    assert(tokens[3].type == TokenType::MULTIPLY);
    assert(tokens[4].type == TokenType::INTEGER);
    assert(tokens[5].type == TokenType::MINUS);
    
    std::cout << "✓ Arithmetic expression test passed" << std::endl;
}

int main()
{
    std::cout << "Running Lexer tests..." << std::endl;
    std::cout << "==========================================\n" << std::endl;
    
    try
    {
        test_basic_tokens();
        test_integers();
        test_floats();
        test_booleans();
        test_strings();
        test_identifiers();
        test_comments();
        test_builtin_types();
        test_color_hex();
        test_special_symbols();
        test_section_header();
        test_key_value_pair();
        test_array_syntax();
        test_inheritance_syntax();
        test_arithmetic_expression();
        
        std::cout << "\n==========================================" << std::endl;
        std::cout << "All tests passed! ✓" << std::endl;
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}
