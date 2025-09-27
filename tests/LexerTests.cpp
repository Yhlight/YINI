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