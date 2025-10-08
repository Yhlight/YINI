#include "doctest/doctest.h"
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "Parser/Ast.h"

TEST_CASE("ParseSimpleSection")
{
    std::string source = "[Section]\nkey = value";
    Yini::Lexer lexer(source);
    std::vector<Yini::Token> tokens = lexer.scanTokens();
    Yini::Parser parser(tokens);
    std::vector<std::unique_ptr<Yini::SectionNode>> ast = parser.parse();

    REQUIRE(ast.size() == 1);

    auto& section = ast[0];
    REQUIRE(section != nullptr);
    CHECK(section->name.lexeme == "Section");

    REQUIRE(section->pairs.size() == 1);

    auto& pair = section->pairs[0];
    REQUIRE(pair != nullptr);
    CHECK(pair->key.lexeme == "key");

    REQUIRE(pair->value != nullptr);
    CHECK(pair->value->token.lexeme == "value");
}