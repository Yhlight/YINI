#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "Lexer/Lexer.h"
#include "Lexer/Token.h"

TEST_CASE("TokenizeSimpleKeyValuePair")
{
    std::string source = "[Section]\nkey = value";
    Yini::Lexer lexer(source);
    std::vector<Yini::Token> tokens = lexer.scanTokens();

    REQUIRE(tokens.size() == 7);
    CHECK(tokens[0].type == Yini::TokenType::LeftBracket);
    CHECK(tokens[1].type == Yini::TokenType::Identifier);
    CHECK(tokens[2].type == Yini::TokenType::RightBracket);
    CHECK(tokens[3].type == Yini::TokenType::Identifier);
    CHECK(tokens[4].type == Yini::TokenType::Equal);
    CHECK(tokens[5].type == Yini::TokenType::Identifier);
    CHECK(tokens[6].type == Yini::TokenType::Eof);
}