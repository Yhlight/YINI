#include "gtest/gtest.h"
#include "YINI/Lexer.h"
#include "YINI/Token.h"
#include <vector>

TEST(LexerSectionsTest, SpecialSections) {
    std::string input = R"(
[Config]
[#define]
[#include]
[#schema]
)";

    YINI::Lexer lexer(input);

    std::vector<YINI::Token> expected_tokens = {
        {YINI::TokenType::Section, "Config", 0, 0},
        {YINI::TokenType::Define, "#define", 0, 0},
        {YINI::TokenType::Include, "#include", 0, 0},
        {YINI::TokenType::Schema, "#schema", 0, 0},
        {YINI::TokenType::Eof, "", 0, 0},
    };

    for (const auto& expected_token : expected_tokens) {
        YINI::Token token = lexer.NextToken();
        ASSERT_EQ(token.type, expected_token.type);
        ASSERT_EQ(token.literal, expected_token.literal);
    }
}