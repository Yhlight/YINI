#include <gtest/gtest.h>
#include "Lexer/lexer.h"

TEST(LexerTest, KeyValuePair) {
    std::string input = "key = \"value\"";
    Lexer lexer(input);

    Token key_token = lexer.nextToken();
    EXPECT_EQ(key_token.type, TokenType::Identifier);
    EXPECT_EQ(key_token.value, "key");
    EXPECT_EQ(key_token.line, 1);
    EXPECT_EQ(key_token.column, 1);

    Token equals_token = lexer.nextToken();
    EXPECT_EQ(equals_token.type, TokenType::Equals);
    EXPECT_EQ(equals_token.line, 1);
    EXPECT_EQ(equals_token.column, 5);

    Token value_token = lexer.nextToken();
    EXPECT_EQ(value_token.type, TokenType::String);
    EXPECT_EQ(value_token.value, "value");
    EXPECT_EQ(value_token.line, 1);
    EXPECT_EQ(value_token.column, 7);
}

TEST(LexerTest, SingleLineComment) {
    std::string input = "// this is a comment\nkey = \"value\"";
    Lexer lexer(input);

    // The lexer should skip the comment and tokenize the next line
    Token key_token = lexer.nextToken();
    EXPECT_EQ(key_token.type, TokenType::Identifier);
    EXPECT_EQ(key_token.value, "key");
    EXPECT_EQ(key_token.line, 2);
    EXPECT_EQ(key_token.column, 1);
}

TEST(LexerTest, MultiLineComment) {
    std::string input = "/* this is a multi-line\n comment */\nkey = \"value\"";
    Lexer lexer(input);

    // The lexer should skip the comment and tokenize the next line
    Token key_token = lexer.nextToken();
    EXPECT_EQ(key_token.type, TokenType::Identifier);
    EXPECT_EQ(key_token.value, "key");
    EXPECT_EQ(key_token.line, 3);
    EXPECT_EQ(key_token.column, 1);
}