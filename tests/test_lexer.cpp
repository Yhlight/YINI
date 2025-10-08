#include <gtest/gtest.h>
#include "Lexer/lexer.h"

TEST(LexerTest, KeyValuePair) {
    std::string input = "key = \"value\"";
    Lexer lexer(input);

    Token key_token = lexer.nextToken();
    EXPECT_EQ(key_token.type, TokenType::Identifier);
    EXPECT_EQ(key_token.value, "key");

    Token equals_token = lexer.nextToken();
    EXPECT_EQ(equals_token.type, TokenType::Equals);

    Token value_token = lexer.nextToken();
    EXPECT_EQ(value_token.type, TokenType::String);
    EXPECT_EQ(value_token.value, "value");
}

TEST(LexerTest, SingleLineComment) {
    std::string input = "// this is a comment\nkey = \"value\"";
    Lexer lexer(input);

    // The lexer should skip the comment and tokenize the next line
    Token key_token = lexer.nextToken();
    EXPECT_EQ(key_token.type, TokenType::Identifier);
    EXPECT_EQ(key_token.value, "key");
}

TEST(LexerTest, MultiLineComment) {
    std::string input = "/* this is a multi-line\n comment */\nkey = \"value\"";
    Lexer lexer(input);

    // The lexer should skip the comment and tokenize the next line
    Token key_token = lexer.nextToken();
    EXPECT_EQ(key_token.type, TokenType::Identifier);
    EXPECT_EQ(key_token.value, "key");
}