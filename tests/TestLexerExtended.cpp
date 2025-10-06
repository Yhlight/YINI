#include "gtest/gtest.h"
#include "YINI/Lexer.h"
#include "YINI/Token.h"
#include <vector>

TEST(LexerTest, ExtendedTokens) {
    std::string input = R"(
/*
  Block comment
*/
key += {1, 2}
another_key: (true - 1) * 5.0 % 2
)";

    YINI::Lexer lexer(input);

    std::vector<YINI::TokenType> expected_types = {
        YINI::TokenType::BlockComment,
        YINI::TokenType::Identifier,
        YINI::TokenType::PlusAssign,
        YINI::TokenType::LeftBrace,
        YINI::TokenType::Integer,
        YINI::TokenType::Comma,
        YINI::TokenType::Integer,
        YINI::TokenType::RightBrace,
        YINI::TokenType::Identifier,
        YINI::TokenType::Colon,
        YINI::TokenType::LeftParen,
        YINI::TokenType::Boolean, // true
        YINI::TokenType::Minus,
        YINI::TokenType::Integer,
        YINI::TokenType::RightParen,
        YINI::TokenType::Asterisk,
        YINI::TokenType::Float,
        YINI::TokenType::Percent,
        YINI::TokenType::Integer,
        YINI::TokenType::Eof,
    };

    for (size_t i = 0; i < expected_types.size(); ++i)
    {
        YINI::Token token = lexer.NextToken();
        ASSERT_EQ(token.type, expected_types[i]);
    }
}