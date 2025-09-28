#include <gtest/gtest.h>
#include "Lexer.h"
#include <string>

TEST(LexerTest, TestSection)
{
    std::string source = "[Config]";
    YINI::Lexer lexer(source);
    std::vector<YINI::Token> tokens = lexer.scanTokens();

    ASSERT_EQ(tokens.size(), 4); // Includes EndOfFile token
    EXPECT_EQ(tokens[0].type, YINI::TokenType::LBracket);
    EXPECT_EQ(tokens[1].type, YINI::TokenType::Identifier);
    EXPECT_EQ(tokens[1].lexeme, "Config");
    EXPECT_EQ(tokens[2].type, YINI::TokenType::RBracket);
    EXPECT_EQ(tokens[3].type, YINI::TokenType::EndOfFile);
}

TEST(LexerTest, TestKeyValueString)
{
    std::string source = "key = \"value\"";
    YINI::Lexer lexer(source);
    std::vector<YINI::Token> tokens = lexer.scanTokens();

    ASSERT_EQ(tokens.size(), 4);
    EXPECT_EQ(tokens[0].type, YINI::TokenType::Identifier);
    EXPECT_EQ(tokens[0].lexeme, "key");
    EXPECT_EQ(tokens[1].type, YINI::TokenType::Equals);
    EXPECT_EQ(tokens[2].type, YINI::TokenType::String);
    EXPECT_EQ(std::get<std::string>(tokens[2].literal), "value");
    EXPECT_EQ(tokens[3].type, YINI::TokenType::EndOfFile);
}

TEST(LexerTest, TestKeyValueNumber)
{
    std::string source = "key_int = 123\nkey_float = 3.14";
    YINI::Lexer lexer(source);
    std::vector<YINI::Token> tokens = lexer.scanTokens();

    ASSERT_EQ(tokens.size(), 7);
    EXPECT_EQ(tokens[0].type, YINI::TokenType::Identifier);
    EXPECT_EQ(tokens[0].lexeme, "key_int");
    EXPECT_EQ(tokens[1].type, YINI::TokenType::Equals);
    EXPECT_EQ(tokens[2].type, YINI::TokenType::Integer);
    EXPECT_EQ(std::get<long long>(tokens[2].literal), 123);

    EXPECT_EQ(tokens[3].type, YINI::TokenType::Identifier);
    EXPECT_EQ(tokens[3].lexeme, "key_float");
    EXPECT_EQ(tokens[4].type, YINI::TokenType::Equals);
    EXPECT_EQ(tokens[5].type, YINI::TokenType::Float);
    EXPECT_EQ(std::get<double>(tokens[5].literal), 3.14);
    EXPECT_EQ(tokens[6].type, YINI::TokenType::EndOfFile);
}

TEST(LexerTest, TestComments)
{
    std::string source = R"(
// This is a single-line comment.
key = "value" // Another comment.
/* This is a
   multi-line comment. */
[Section]
)";
    YINI::Lexer lexer(source);
    std::vector<YINI::Token> tokens = lexer.scanTokens();

    ASSERT_EQ(tokens.size(), 7);
    EXPECT_EQ(tokens[0].type, YINI::TokenType::Identifier);
    EXPECT_EQ(tokens[1].type, YINI::TokenType::Equals);
    EXPECT_EQ(tokens[2].type, YINI::TokenType::String);
    EXPECT_EQ(tokens[3].type, YINI::TokenType::LBracket);
    EXPECT_EQ(tokens[4].type, YINI::TokenType::Identifier);
    EXPECT_EQ(tokens[5].type, YINI::TokenType::RBracket);
    EXPECT_EQ(tokens[6].type, YINI::TokenType::EndOfFile);
}

TEST(LexerTest, TestAllOperatorsAndPunctuation)
{
    std::string source = "+-*/%{}() ,:#@";
    YINI::Lexer lexer(source);
    std::vector<YINI::Token> tokens = lexer.scanTokens();

    ASSERT_EQ(tokens.size(), 14);
    EXPECT_EQ(tokens[0].type, YINI::TokenType::Plus);
    EXPECT_EQ(tokens[1].type, YINI::TokenType::Minus);
    EXPECT_EQ(tokens[2].type, YINI::TokenType::Star);
    EXPECT_EQ(tokens[3].type, YINI::TokenType::Slash);
    EXPECT_EQ(tokens[4].type, YINI::TokenType::Percent);
    EXPECT_EQ(tokens[5].type, YINI::TokenType::LBrace);
    EXPECT_EQ(tokens[6].type, YINI::TokenType::RBrace);
    EXPECT_EQ(tokens[7].type, YINI::TokenType::LParen);
    EXPECT_EQ(tokens[8].type, YINI::TokenType::RParen);
    EXPECT_EQ(tokens[9].type, YINI::TokenType::Comma);
    EXPECT_EQ(tokens[10].type, YINI::TokenType::Colon);
    EXPECT_EQ(tokens[11].type, YINI::TokenType::Hash);
    EXPECT_EQ(tokens[12].type, YINI::TokenType::At);
    EXPECT_EQ(tokens[13].type, YINI::TokenType::EndOfFile);
}

TEST(LexerTest, TestKeywords)
{
    std::string source = "true false Coord Color Path Dyna";
    YINI::Lexer lexer(source);
    std::vector<YINI::Token> tokens = lexer.scanTokens();

    ASSERT_EQ(tokens.size(), 7);
    EXPECT_EQ(tokens[0].type, YINI::TokenType::True);
    EXPECT_EQ(tokens[1].type, YINI::TokenType::False);
    EXPECT_EQ(tokens[2].type, YINI::TokenType::Coord);
    EXPECT_EQ(tokens[3].type, YINI::TokenType::Color);
    EXPECT_EQ(tokens[4].type, YINI::TokenType::Path);
    EXPECT_EQ(tokens[5].type, YINI::TokenType::Dyna);
    EXPECT_EQ(tokens[6].type, YINI::TokenType::EndOfFile);
}