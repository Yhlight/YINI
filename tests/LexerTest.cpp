#include <gtest/gtest.h>
#include "Lexer/Lexer.h"
#include <gtest/gtest.h>
#include <vector>
#include <variant>

TEST(LexerTest, TokenizesSimpleInput)
{
    std::string source = "[Section]\nkey = \"value\"";
    YINI::Lexer lexer(source);
    std::vector<YINI::Token> tokens = lexer.scanTokens();

    ASSERT_EQ(tokens.size(), 7);

    EXPECT_EQ(tokens[0].type, YINI::TokenType::LEFT_BRACKET);
    EXPECT_EQ(tokens[1].type, YINI::TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[2].type, YINI::TokenType::RIGHT_BRACKET);
    EXPECT_EQ(tokens[3].type, YINI::TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[4].type, YINI::TokenType::EQUAL);
    EXPECT_EQ(tokens[5].type, YINI::TokenType::STRING);
    EXPECT_EQ(tokens[6].type, YINI::TokenType::END_OF_FILE);
}

TEST(LexerTest, TokenizesDataTypesAndComments)
{
    std::string source = R"(
        // This is a comment.
        key_int = 123
        key_float = 3.14
        /* This is a
           block comment. */
        key_true = true
        key_false = false
    )";

    YINI::Lexer lexer(source);
    std::vector<YINI::Token> tokens = lexer.scanTokens();

    // The lexer should now produce 15 tokens:
    // 1 comment, 3 for int, 3 for float, 1 comment, 3 for true, 3 for false, 1 EOF
    ASSERT_EQ(tokens.size(), 15);

    // First comment
    EXPECT_EQ(tokens[0].type, YINI::TokenType::COMMENT);
    EXPECT_EQ(std::get<std::string>(tokens[0].literal.m_value), " This is a comment.");

    // key_int = 123
    EXPECT_EQ(tokens[1].type, YINI::TokenType::IDENTIFIER);
    EXPECT_EQ(std::get<std::string>(tokens[1].literal.m_value), "key_int");
    EXPECT_EQ(tokens[2].type, YINI::TokenType::EQUAL);
    EXPECT_EQ(tokens[3].type, YINI::TokenType::NUMBER);
    EXPECT_EQ(std::get<double>(tokens[3].literal.m_value), 123);

    // key_float = 3.14
    EXPECT_EQ(tokens[4].type, YINI::TokenType::IDENTIFIER);
    EXPECT_EQ(std::get<std::string>(tokens[4].literal.m_value), "key_float");
    EXPECT_EQ(tokens[5].type, YINI::TokenType::EQUAL);
    EXPECT_EQ(tokens[6].type, YINI::TokenType::NUMBER);
    EXPECT_EQ(std::get<double>(tokens[6].literal.m_value), 3.14);

    // Block comment
    EXPECT_EQ(tokens[7].type, YINI::TokenType::COMMENT);
    EXPECT_EQ(std::get<std::string>(tokens[7].literal.m_value), " This is a\n           block comment. ");

    // key_true = true
    EXPECT_EQ(tokens[8].type, YINI::TokenType::IDENTIFIER);
    EXPECT_EQ(std::get<std::string>(tokens[8].literal.m_value), "key_true");
    EXPECT_EQ(tokens[9].type, YINI::TokenType::EQUAL);
    EXPECT_EQ(tokens[10].type, YINI::TokenType::TRUE);
    EXPECT_EQ(std::get<bool>(tokens[10].literal.m_value), true);

    // key_false = false
    EXPECT_EQ(tokens[11].type, YINI::TokenType::IDENTIFIER);
    EXPECT_EQ(std::get<std::string>(tokens[11].literal.m_value), "key_false");
    EXPECT_EQ(tokens[12].type, YINI::TokenType::EQUAL);
    EXPECT_EQ(tokens[13].type, YINI::TokenType::FALSE);
    EXPECT_EQ(std::get<bool>(tokens[13].literal.m_value), false);

    EXPECT_EQ(tokens[14].type, YINI::TokenType::END_OF_FILE);
}