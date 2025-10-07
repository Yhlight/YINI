#include <gtest/gtest.h>
#include "Lexer/Lexer.h"

using namespace YINI;

TEST(LexerTest, BasicTokens) {
    std::string source = "[](){},:=+=";
    Lexer lexer(source);

    Token token = lexer.nextToken();
    EXPECT_EQ(token.type, TokenType::LeftBracket);
    EXPECT_EQ(token.text, "[");

    token = lexer.nextToken();
    EXPECT_EQ(token.type, TokenType::RightBracket);
    EXPECT_EQ(token.text, "]");

    token = lexer.nextToken();
    EXPECT_EQ(token.type, TokenType::LeftParen);
    EXPECT_EQ(token.text, "(");

    token = lexer.nextToken();
    EXPECT_EQ(token.type, TokenType::RightParen);
    EXPECT_EQ(token.text, ")");

    token = lexer.nextToken();
    EXPECT_EQ(token.type, TokenType::LeftBrace);
    EXPECT_EQ(token.text, "{");

    token = lexer.nextToken();
    EXPECT_EQ(token.type, TokenType::RightBrace);
    EXPECT_EQ(token.text, "}");

    token = lexer.nextToken();
    EXPECT_EQ(token.type, TokenType::Comma);
    EXPECT_EQ(token.text, ",");

    token = lexer.nextToken();
    EXPECT_EQ(token.type, TokenType::Colon);
    EXPECT_EQ(token.text, ":");

    token = lexer.nextToken();
    EXPECT_EQ(token.type, TokenType::Equals);
    EXPECT_EQ(token.text, "=");

    token = lexer.nextToken();
    EXPECT_EQ(token.type, TokenType::PlusEquals);
    EXPECT_EQ(token.text, "+=");

    token = lexer.nextToken();
    EXPECT_EQ(token.type, TokenType::EndOfFile);
}

TEST(LexerTest, IdentifierAndKeywords) {
    std::string source = "identifier true false";
    Lexer lexer(source);

    Token token = lexer.nextToken();
    EXPECT_EQ(token.type, TokenType::Identifier);
    EXPECT_EQ(token.text, "identifier");

    token = lexer.nextToken();
    EXPECT_EQ(token.type, TokenType::True);
    EXPECT_EQ(token.text, "true");

    token = lexer.nextToken();
    EXPECT_EQ(token.type, TokenType::False);
    EXPECT_EQ(token.text, "false");
}

TEST(LexerTest, Numbers) {
    std::string source = "123 3.14";
    Lexer lexer(source);

    Token token = lexer.nextToken();
    EXPECT_EQ(token.type, TokenType::Integer);
    EXPECT_EQ(token.text, "123");

    token = lexer.nextToken();
    EXPECT_EQ(token.type, TokenType::Float);
    EXPECT_EQ(token.text, "3.14");
}

TEST(LexerTest, String) {
    std::string source = "\"hello world\"";
    Lexer lexer(source);

    Token token = lexer.nextToken();
    EXPECT_EQ(token.type, TokenType::String);
    EXPECT_EQ(token.text, "hello world");
}

TEST(LexerTest, Comments) {
    std::string source = R"(
        // This is a single-line comment.
        key = value // Another comment
        /* This is a
         * multi-line comment.
         */
        another_key = "some string"
    )";
    Lexer lexer(source);

    Token token = lexer.nextToken();
    EXPECT_EQ(token.type, TokenType::Identifier);
    EXPECT_EQ(token.text, "key");

    token = lexer.nextToken();
    EXPECT_EQ(token.type, TokenType::Equals);
    EXPECT_EQ(token.text, "=");

    token = lexer.nextToken();
    EXPECT_EQ(token.type, TokenType::Identifier);
    EXPECT_EQ(token.text, "value");

    token = lexer.nextToken();
    EXPECT_EQ(token.type, TokenType::Identifier);
    EXPECT_EQ(token.text, "another_key");

    token = lexer.nextToken();
    EXPECT_EQ(token.type, TokenType::Equals);
    EXPECT_EQ(token.text, "=");

    token = lexer.nextToken();
    EXPECT_EQ(token.type, TokenType::String);
    EXPECT_EQ(token.text, "some string");

    token = lexer.nextToken();
    EXPECT_EQ(token.type, TokenType::EndOfFile);
}