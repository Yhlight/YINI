#include <gtest/gtest.h>
#include "Lexer/Lexer.h"
#include <vector>
#include <any>

TEST(LexerTest, TokenizesSimpleInput)
{
    std::string source = "[Section]\nkey = \"value\"";
    YINI::Lexer lexer(source);
    std::vector<YINI::Token> tokens = lexer.scanTokens();

    ASSERT_EQ(tokens.size(), 7);

    EXPECT_EQ(tokens[0].type, YINI::TokenType::LEFT_BRACKET);
    EXPECT_EQ(tokens[0].lexeme, "[");

    EXPECT_EQ(tokens[1].type, YINI::TokenType::IDENTIFIER);
    EXPECT_EQ(std::any_cast<std::string>(tokens[1].literal), "Section");

    EXPECT_EQ(tokens[2].type, YINI::TokenType::RIGHT_BRACKET);
    EXPECT_EQ(tokens[2].lexeme, "]");

    EXPECT_EQ(tokens[3].type, YINI::TokenType::IDENTIFIER);
    EXPECT_EQ(std::any_cast<std::string>(tokens[3].literal), "key");

    EXPECT_EQ(tokens[4].type, YINI::TokenType::EQUAL);
    EXPECT_EQ(tokens[4].lexeme, "=");

    EXPECT_EQ(tokens[5].type, YINI::TokenType::STRING);
    EXPECT_EQ(std::any_cast<std::string>(tokens[5].literal), "value");

    EXPECT_EQ(tokens[6].type, YINI::TokenType::END_OF_FILE);
}