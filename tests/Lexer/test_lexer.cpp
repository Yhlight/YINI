#include <gtest/gtest.h>
#include "Lexer.h"
#include "Token.h"

using namespace yini;

// Test fixture for Lexer tests to reduce boilerplate
class LexerTest : public ::testing::Test {
protected:
    std::vector<Token> tokenize(const std::string& source) {
        Lexer lexer(source);
        return lexer.tokenize();
    }
};

TEST_F(LexerTest, BasicTokens) {
    auto tokens = tokenize("[ ] ( ) { } , : = += + - * / %");
    ASSERT_EQ(tokens[0].type, TokenType::LBRACKET);
    ASSERT_EQ(tokens[1].type, TokenType::RBRACKET);
    ASSERT_EQ(tokens[2].type, TokenType::LPAREN);
    ASSERT_EQ(tokens[3].type, TokenType::RPAREN);
    ASSERT_EQ(tokens[4].type, TokenType::LBRACE);
    ASSERT_EQ(tokens[5].type, TokenType::RBRACE);
    ASSERT_EQ(tokens[6].type, TokenType::COMMA);
    ASSERT_EQ(tokens[7].type, TokenType::COLON);
    ASSERT_EQ(tokens[8].type, TokenType::EQUALS);
    ASSERT_EQ(tokens[9].type, TokenType::PLUS_EQUALS);
    ASSERT_EQ(tokens[10].type, TokenType::PLUS);
    ASSERT_EQ(tokens[11].type, TokenType::MINUS);
    ASSERT_EQ(tokens[12].type, TokenType::MULTIPLY);
    ASSERT_EQ(tokens[13].type, TokenType::DIVIDE);
    ASSERT_EQ(tokens[14].type, TokenType::MODULO);
    ASSERT_EQ(tokens[15].type, TokenType::END_OF_FILE);
}

TEST_F(LexerTest, Integers) {
    auto tokens = tokenize("123 456 0 999");
    ASSERT_EQ(tokens[0].type, TokenType::INTEGER);
    ASSERT_EQ(tokens[0].getValue<int64_t>(), 123);
    
    ASSERT_EQ(tokens[1].type, TokenType::INTEGER);
    ASSERT_EQ(tokens[1].getValue<int64_t>(), 456);
    
    ASSERT_EQ(tokens[2].type, TokenType::INTEGER);
    ASSERT_EQ(tokens[2].getValue<int64_t>(), 0);
}

TEST_F(LexerTest, Floats) {
    auto tokens = tokenize("3.14 2.5 0.1");
    ASSERT_EQ(tokens[0].type, TokenType::FLOAT);
    ASSERT_EQ(tokens[0].getValue<double>(), 3.14);
    
    ASSERT_EQ(tokens[1].type, TokenType::FLOAT);
    ASSERT_EQ(tokens[1].getValue<double>(), 2.5);
}

TEST_F(LexerTest, Booleans) {
    auto tokens = tokenize("true false");
    ASSERT_EQ(tokens[0].type, TokenType::BOOLEAN);
    ASSERT_EQ(tokens[0].getValue<bool>(), true);
    
    ASSERT_EQ(tokens[1].type, TokenType::BOOLEAN);
    ASSERT_EQ(tokens[1].getValue<bool>(), false);
}

TEST_F(LexerTest, Strings) {
    auto tokens = tokenize(R"("hello" "world" "test\nvalue")");
    ASSERT_EQ(tokens[0].type, TokenType::STRING);
    ASSERT_EQ(tokens[0].getValue<std::string>(), "hello");
    
    ASSERT_EQ(tokens[1].type, TokenType::STRING);
    ASSERT_EQ(tokens[1].getValue<std::string>(), "world");
    
    ASSERT_EQ(tokens[2].type, TokenType::STRING);
    ASSERT_EQ(tokens[2].getValue<std::string>(), "test\nvalue");
}

TEST_F(LexerTest, Identifiers) {
    auto tokens = tokenize("key1 value name_test");
    ASSERT_EQ(tokens[0].type, TokenType::IDENTIFIER);
    ASSERT_EQ(tokens[0].getValue<std::string>(), "key1");
    
    ASSERT_EQ(tokens[1].type, TokenType::IDENTIFIER);
    ASSERT_EQ(tokens[1].getValue<std::string>(), "value");
}

TEST_F(LexerTest, Comments) {
    auto tokens = tokenize("key1 // this is a comment\nkey2 /* block comment */ key3");
    // Comments should be filtered out, but newlines are kept
    ASSERT_EQ(tokens[0].type, TokenType::IDENTIFIER);
    ASSERT_EQ(tokens[0].getValue<std::string>(), "key1");
    
    ASSERT_EQ(tokens[1].type, TokenType::NEWLINE);
    
    ASSERT_EQ(tokens[2].type, TokenType::IDENTIFIER);
    ASSERT_EQ(tokens[2].getValue<std::string>(), "key2");
    
    ASSERT_EQ(tokens[3].type, TokenType::IDENTIFIER);
    ASSERT_EQ(tokens[3].getValue<std::string>(), "key3");
}

TEST_F(LexerTest, BuiltinTypes) {
    auto tokens = tokenize("Color color Coord coord List list Array array Dyna dyna Path path");
    ASSERT_EQ(tokens[0].type, TokenType::COLOR);
    ASSERT_EQ(tokens[1].type, TokenType::COLOR);
    ASSERT_EQ(tokens[2].type, TokenType::COORD);
    ASSERT_EQ(tokens[3].type, TokenType::COORD);
    ASSERT_EQ(tokens[4].type, TokenType::LIST);
    ASSERT_EQ(tokens[5].type, TokenType::LIST);
    ASSERT_EQ(tokens[6].type, TokenType::ARRAY);
    ASSERT_EQ(tokens[7].type, TokenType::ARRAY);
    ASSERT_EQ(tokens[8].type, TokenType::DYNA);
    ASSERT_EQ(tokens[9].type, TokenType::DYNA);
    ASSERT_EQ(tokens[10].type, TokenType::PATH);
    ASSERT_EQ(tokens[11].type, TokenType::PATH);
}

TEST_F(LexerTest, ColorHex) {
    auto tokens = tokenize("#FF0000 #00FF00 #0000FF");
    ASSERT_EQ(tokens[0].type, TokenType::COLOR);
    ASSERT_EQ(tokens[0].getValue<std::string>(), "#FF0000");
    
    ASSERT_EQ(tokens[1].type, TokenType::COLOR);
    ASSERT_EQ(tokens[1].getValue<std::string>(), "#00FF00");
}

TEST_F(LexerTest, SpecialSymbols) {
    auto tokens = tokenize("@ @{ ${ # ! ? ~");
    ASSERT_EQ(tokens[0].type, TokenType::AT);
    ASSERT_EQ(tokens[1].type, TokenType::AT_LBRACE);
    ASSERT_EQ(tokens[2].type, TokenType::DOLLAR_LBRACE);
    ASSERT_EQ(tokens[3].type, TokenType::HASH);
    ASSERT_EQ(tokens[4].type, TokenType::EXCLAMATION);
    ASSERT_EQ(tokens[5].type, TokenType::QUESTION);
    ASSERT_EQ(tokens[6].type, TokenType::TILDE);
}

TEST_F(LexerTest, SectionHeader) {
    auto tokens = tokenize("[Config]");
    ASSERT_EQ(tokens[0].type, TokenType::LBRACKET);
    ASSERT_EQ(tokens[1].type, TokenType::IDENTIFIER);
    ASSERT_EQ(tokens[1].getValue<std::string>(), "Config");
    ASSERT_EQ(tokens[2].type, TokenType::RBRACKET);
}

TEST_F(LexerTest, KeyValuePair) {
    auto tokens = tokenize("key = value");
    ASSERT_EQ(tokens[0].type, TokenType::IDENTIFIER);
    ASSERT_EQ(tokens[0].getValue<std::string>(), "key");
    ASSERT_EQ(tokens[1].type, TokenType::EQUALS);
    ASSERT_EQ(tokens[2].type, TokenType::IDENTIFIER);
    ASSERT_EQ(tokens[2].getValue<std::string>(), "value");
}

TEST_F(LexerTest, ArraySyntax) {
    auto tokens = tokenize("[1, 2, 3]");
    ASSERT_EQ(tokens[0].type, TokenType::LBRACKET);
    ASSERT_EQ(tokens[1].type, TokenType::INTEGER);
    ASSERT_EQ(tokens[2].type, TokenType::COMMA);
    ASSERT_EQ(tokens[3].type, TokenType::INTEGER);
    ASSERT_EQ(tokens[4].type, TokenType::COMMA);
    ASSERT_EQ(tokens[5].type, TokenType::INTEGER);
    ASSERT_EQ(tokens[6].type, TokenType::RBRACKET);
}

TEST_F(LexerTest, InheritanceSyntax) {
    auto tokens = tokenize("[Config3] : Config, Config2");
    ASSERT_EQ(tokens[0].type, TokenType::LBRACKET);
    ASSERT_EQ(tokens[1].type, TokenType::IDENTIFIER);
    ASSERT_EQ(tokens[1].getValue<std::string>(), "Config3");
    ASSERT_EQ(tokens[2].type, TokenType::RBRACKET);
    ASSERT_EQ(tokens[3].type, TokenType::COLON);
    ASSERT_EQ(tokens[4].type, TokenType::IDENTIFIER);
    ASSERT_EQ(tokens[4].getValue<std::string>(), "Config");
}

TEST_F(LexerTest, ArithmeticExpression) {
    auto tokens = tokenize("1 + 2 * 3 - 4 / 5 % 6");
    ASSERT_EQ(tokens[0].type, TokenType::INTEGER);
    ASSERT_EQ(tokens[1].type, TokenType::PLUS);
    ASSERT_EQ(tokens[2].type, TokenType::INTEGER);
    ASSERT_EQ(tokens[3].type, TokenType::MULTIPLY);
    ASSERT_EQ(tokens[4].type, TokenType::INTEGER);
    ASSERT_EQ(tokens[5].type, TokenType::MINUS);
}

// Test suite for lexer error conditions
class LexerErrorTest : public ::testing::Test {};

TEST_F(LexerErrorTest, UnterminatedString) {
    Lexer lexer(R"("hello world)");
    lexer.tokenize(); // Run the lexer
    ASSERT_TRUE(lexer.hasError());
    ASSERT_NE(lexer.getLastError().find("Unterminated string"), std::string::npos);
}

TEST_F(LexerErrorTest, UnterminatedBlockComment) {
    Lexer lexer("key = 1 /* this is a comment");
    lexer.tokenize();
    ASSERT_TRUE(lexer.hasError());
    ASSERT_NE(lexer.getLastError().find("Unterminated block comment"), std::string::npos);
}

TEST_F(LexerErrorTest, UnexpectedCharacter) {
    Lexer lexer("key = ^");
    lexer.tokenize();
    ASSERT_TRUE(lexer.hasError());
    ASSERT_NE(lexer.getLastError().find("Unexpected character"), std::string::npos);
}

TEST_F(LexerErrorTest, IncompleteDollarBrace) {
    Lexer lexer("key = $");
    lexer.tokenize();
    ASSERT_TRUE(lexer.hasError());
    ASSERT_NE(lexer.getLastError().find("Expected '{' after '$'"), std::string::npos);
}