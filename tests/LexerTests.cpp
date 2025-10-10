#include "gtest/gtest.h"
#include "Lexer/Lexer.h"
#include "Lexer/Token.h"

TEST(LexerTests, TokenizesSingleTokens) {
    std::string source = "+-(){}[],.:;=";
    YINI::Lexer lexer(source);
    std::vector<YINI::Token> tokens = lexer.scan_tokens();

    ASSERT_EQ(tokens.size(), 14);
    EXPECT_EQ(tokens[0].type, YINI::TokenType::PLUS);
    EXPECT_EQ(tokens[1].type, YINI::TokenType::MINUS);
    EXPECT_EQ(tokens[2].type, YINI::TokenType::LEFT_PAREN);
    EXPECT_EQ(tokens[3].type, YINI::TokenType::RIGHT_PAREN);
    EXPECT_EQ(tokens[4].type, YINI::TokenType::LEFT_BRACE);
    EXPECT_EQ(tokens[5].type, YINI::TokenType::RIGHT_BRACE);
    EXPECT_EQ(tokens[6].type, YINI::TokenType::LEFT_BRACKET);
    EXPECT_EQ(tokens[7].type, YINI::TokenType::RIGHT_BRACKET);
    EXPECT_EQ(tokens[8].type, YINI::TokenType::COMMA);
    EXPECT_EQ(tokens[9].type, YINI::TokenType::DOT);
    EXPECT_EQ(tokens[10].type, YINI::TokenType::COLON);
    EXPECT_EQ(tokens[11].type, YINI::TokenType::SEMICOLON);
    EXPECT_EQ(tokens[12].type, YINI::TokenType::EQUAL);
    EXPECT_EQ(tokens[13].type, YINI::TokenType::END_OF_FILE);
}

TEST(LexerTests, TokenizesAssignment) {
    std::string source = "key = \"value\"";
    YINI::Lexer lexer(source);
    std::vector<YINI::Token> tokens = lexer.scan_tokens();

    ASSERT_EQ(tokens.size(), 4);
    EXPECT_EQ(tokens[0].type, YINI::TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[0].lexeme, "key");
    EXPECT_EQ(tokens[1].type, YINI::TokenType::EQUAL);
    EXPECT_EQ(tokens[2].type, YINI::TokenType::STRING);
    EXPECT_EQ(std::get<std::string>(tokens[2].literal), "value");
}

TEST(LexerTests, TokenizesKeywords) {
    std::string source = "true false color";
    YINI::Lexer lexer(source);
    std::vector<YINI::Token> tokens = lexer.scan_tokens();

    ASSERT_EQ(tokens.size(), 4);
    EXPECT_EQ(tokens[0].type, YINI::TokenType::TRUE);
    EXPECT_EQ(tokens[1].type, YINI::TokenType::FALSE);
    EXPECT_EQ(tokens[2].type, YINI::TokenType::COLOR);
    EXPECT_EQ(tokens[3].type, YINI::TokenType::END_OF_FILE);
}

TEST(LexerTests, IgnoresComments) {
    std::string source = "// this is a comment\n"
                         "key = value /* this is another comment */";
    YINI::Lexer lexer(source);
    std::vector<YINI::Token> tokens = lexer.scan_tokens();

    ASSERT_EQ(tokens.size(), 4);
    EXPECT_EQ(tokens[0].type, YINI::TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[0].lexeme, "key");
    EXPECT_EQ(tokens[1].type, YINI::TokenType::EQUAL);
    EXPECT_EQ(tokens[2].type, YINI::TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[2].lexeme, "value");
    EXPECT_EQ(tokens[3].type, YINI::TokenType::END_OF_FILE);
}

TEST(LexerTests, HandlesUnterminatedBlockComment) {
    std::string source = "/* this is an unterminated comment";
    YINI::Lexer lexer(source);
    std::vector<YINI::Token> tokens = lexer.scan_tokens();

    ASSERT_EQ(tokens.size(), 1);
    EXPECT_EQ(tokens[0].type, YINI::TokenType::END_OF_FILE);
}

TEST(LexerTests, TokenizesSection) {
    std::string source = "[Section]";
    YINI::Lexer lexer(source);
    std::vector<YINI::Token> tokens = lexer.scan_tokens();

    ASSERT_EQ(tokens.size(), 4);
    EXPECT_EQ(tokens[0].type, YINI::TokenType::LEFT_BRACKET);
    EXPECT_EQ(tokens[1].type, YINI::TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[1].lexeme, "Section");
    EXPECT_EQ(tokens[2].type, YINI::TokenType::RIGHT_BRACKET);
    EXPECT_EQ(tokens[3].type, YINI::TokenType::END_OF_FILE);
}