#include "gtest/gtest.h"
#include "YINI/Lexer.h"
#include "YINI/Token.h"
#include <vector>

TEST(LexerLiteralsTest, BooleanLiterals) {
    std::string input = "is_enabled = true; is_disabled = false;";
    YINI::Lexer lexer(input);

    std::vector<YINI::Token> expected_tokens = {
        {YINI::TokenType::Identifier, "is_enabled", 1, 0},
        {YINI::TokenType::Assign, "=", 1, 0},
        {YINI::TokenType::Boolean, "true", 1, 0},
        {YINI::TokenType::Illegal, ";", 1, 0}, // Semicolon is not a valid token yet
        {YINI::TokenType::Identifier, "is_disabled", 1, 0},
        {YINI::TokenType::Assign, "=", 1, 0},
        {YINI::TokenType::Boolean, "false", 1, 0},
        {YINI::TokenType::Illegal, ";", 1, 0},
        {YINI::TokenType::Eof, "", 1, 0},
    };

    for (const auto& expected_token : expected_tokens) {
        YINI::Token token = lexer.NextToken();
        ASSERT_EQ(token.type, expected_token.type);
        ASSERT_EQ(token.literal, expected_token.literal);
    }
}