#include <gtest/gtest.h>
#include "Lexer/Lexer.h"
#include "Lexer/Token.h"
#include <vector>

TEST(LexerTest, SkipsCommentsAndFindsNextToken)
{
    std::string input = R"(
        // This is a single-line comment.
        /* This is a
           multi-line comment. */

        ident // Another comment
    )";

    YINI::Lexer lexer(input);

    std::vector<YINI::TokenType> expected_tokens = {
        YINI::TokenType::Identifier,
        YINI::TokenType::EndOfFile
    };

    for(const auto& expected_type : expected_tokens) {
        YINI::Token token = lexer.nextToken();
        EXPECT_EQ(token.type, expected_type);
    }
}

TEST(LexerTest, TokenizesSections)
{
    std::string input = "[SectionName]";
    YINI::Lexer lexer(input);

    std::vector<YINI::Token> expected_tokens = {
        {YINI::TokenType::LBracket, "[", 1, 1},
        {YINI::TokenType::Identifier, "SectionName", 1, 2},
        {YINI::TokenType::RBracket, "]", 1, 13},
        {YINI::TokenType::EndOfFile, "", 1, 14}
    };

    for(const auto& expected_token : expected_tokens) {
        YINI::Token token = lexer.nextToken();
        EXPECT_EQ(token.type, expected_token.type);
        EXPECT_EQ(token.literal, expected_token.literal);
    }
}

TEST(LexerTest, TokenizesPlusAssign)
{
    std::string input = "+=";
    YINI::Lexer lexer(input);

    std::vector<YINI::Token> expected_tokens = {
        {YINI::TokenType::PlusAssign, "+=", 1, 1},
        {YINI::TokenType::EndOfFile, "", 1, 3}
    };

    for(const auto& expected_token : expected_tokens) {
        YINI::Token token = lexer.nextToken();
        EXPECT_EQ(token.type, expected_token.type);
        EXPECT_EQ(token.literal, expected_token.literal);
    }
}

TEST(LexerTest, TokenizesOperators)
{
    std::string input = "+-*/%()";
    YINI::Lexer lexer(input);

    std::vector<YINI::Token> expected_tokens = {
        {YINI::TokenType::Plus, "+", 1, 1},
        {YINI::TokenType::Minus, "-", 1, 2},
        {YINI::TokenType::Asterisk, "*", 1, 3},
        {YINI::TokenType::Slash, "/", 1, 4},
        {YINI::TokenType::Percent, "%", 1, 5},
        {YINI::TokenType::LParen, "(", 1, 6},
        {YINI::TokenType::RParen, ")", 1, 7},
        {YINI::TokenType::EndOfFile, "", 1, 8}
    };

    for(const auto& expected_token : expected_tokens) {
        YINI::Token token = lexer.nextToken();
        EXPECT_EQ(token.type, expected_token.type);
        EXPECT_EQ(token.literal, expected_token.literal);
    }
}

TEST(LexerTest, TokenizesBooleans)
{
    std::string input = "true false";
    YINI::Lexer lexer(input);

    std::vector<YINI::Token> expected_tokens = {
        {YINI::TokenType::True, "true", 1, 1},
        {YINI::TokenType::False, "false", 1, 6},
        {YINI::TokenType::EndOfFile, "", 1, 11}
    };

    for(const auto& expected_token : expected_tokens) {
        YINI::Token token = lexer.nextToken();
        EXPECT_EQ(token.type, expected_token.type);
        EXPECT_EQ(token.literal, expected_token.literal);
    }
}

TEST(LexerTest, TokenizesNumbers)
{
    std::string input = "123 3.14";
    YINI::Lexer lexer(input);

    std::vector<YINI::Token> expected_tokens = {
        {YINI::TokenType::Integer, "123", 1, 1},
        {YINI::TokenType::Float, "3.14", 1, 5},
        {YINI::TokenType::EndOfFile, "", 1, 9}
    };

    for(const auto& expected_token : expected_tokens) {
        YINI::Token token = lexer.nextToken();
        EXPECT_EQ(token.type, expected_token.type);
        EXPECT_EQ(token.literal, expected_token.literal);
    }
}

TEST(LexerTest, TokenizesKeyValuePairs)
{
    std::string input = R"(key = "value")";
    YINI::Lexer lexer(input);

    std::vector<YINI::Token> expected_tokens = {
        {YINI::TokenType::Identifier, "key", 1, 1},
        {YINI::TokenType::Assign, "=", 1, 5},
        {YINI::TokenType::String, "value", 1, 7},
        {YINI::TokenType::EndOfFile, "", 1, 14}
    };

    for(const auto& expected_token : expected_tokens) {
        YINI::Token token = lexer.nextToken();
        EXPECT_EQ(token.type, expected_token.type);
        EXPECT_EQ(token.literal, expected_token.literal);
    }
}