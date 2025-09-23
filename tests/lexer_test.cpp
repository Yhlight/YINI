#include <gtest/gtest.h>
#include "Lexer/Lexer.h"
#include <vector>

using namespace Yini;

TEST(LexerTest, TestBasicTokens) {
    std::string input = "[]:+=,=@*/%()";
    Lexer lexer(input);

    std::vector<TokenType> expected_types = {
        TokenType::LBracket, TokenType::RBracket, TokenType::Colon, TokenType::PlusAssign,
        TokenType::Assign, TokenType::Comma, TokenType::At, TokenType::Star, TokenType::Slash,
        TokenType::Percent, TokenType::LParen, TokenType::RParen, TokenType::Eof
    };

    for (const auto& expected_type : expected_types) {
        Token tok = lexer.nextToken();
        ASSERT_EQ(tok.type, expected_type);
    }
}

TEST(LexerTest, TestSectionAndKeyValuePair) {
    std::string input = R"yini(
[Section1]
key1 = "value1"
key2 = 123
key3 = 3.14
key4 = true
)yini";
    Lexer lexer(input);

    std::vector<Token> expected_tokens = {
        {TokenType::LBracket, "["},
        {TokenType::Identifier, "Section1"},
        {TokenType::RBracket, "]"},
        {TokenType::Identifier, "key1"},
        {TokenType::Assign, "="},
        {TokenType::String, "value1"},
        {TokenType::Identifier, "key2"},
        {TokenType::Assign, "="},
        {TokenType::Integer, "123"},
        {TokenType::Identifier, "key3"},
        {TokenType::Assign, "="},
        {TokenType::Float, "3.14"},
        {TokenType::Identifier, "key4"},
        {TokenType::Assign, "="},
        {TokenType::True, "true"},
        {TokenType::Eof, ""}
    };

    for (const auto& expected_tok : expected_tokens) {
        Token tok = lexer.nextToken();
        ASSERT_EQ(tok.type, expected_tok.type);
        ASSERT_EQ(tok.literal, expected_tok.literal);
    }
}

TEST(LexerTest, TestComments) {
    std::string input = R"yini(
// This is a single line comment
[Section] // Another comment
/* This is a
   block comment */
key = value /* comment after value */
)yini";
    Lexer lexer(input);

    std::vector<Token> expected_tokens = {
        {TokenType::LBracket, "["},
        {TokenType::Identifier, "Section"},
        {TokenType::RBracket, "]"},
        {TokenType::Identifier, "key"},
        {TokenType::Assign, "="},
        {TokenType::Identifier, "value"},
        {TokenType::Eof, ""}
    };

    for (const auto& expected_tok : expected_tokens) {
        Token tok = lexer.nextToken();
        ASSERT_EQ(tok.type, expected_tok.type);
        ASSERT_EQ(tok.literal, expected_tok.literal);
    }
}

TEST(LexerTest, TestKeywordsCaseInsensitive) {
    std::string input = "Coord COORD cOoRd Dyna dYnA";
    Lexer lexer(input);

    ASSERT_EQ(lexer.nextToken().type, TokenType::Coord);
    ASSERT_EQ(lexer.nextToken().type, TokenType::Coord);
    ASSERT_EQ(lexer.nextToken().type, TokenType::Coord);
    ASSERT_EQ(lexer.nextToken().type, TokenType::Dyna);
    ASSERT_EQ(lexer.nextToken().type, TokenType::Dyna);
    ASSERT_EQ(lexer.nextToken().type, TokenType::Eof);
}
