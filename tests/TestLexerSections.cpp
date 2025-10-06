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

    std::vector<YINI::TokenType> expected_types = {
        YINI::TokenType::LeftBracket,
        YINI::TokenType::Identifier,
        YINI::TokenType::RightBracket,
        YINI::TokenType::LeftBracket,
        YINI::TokenType::Identifier,
        YINI::TokenType::RightBracket,
        YINI::TokenType::LeftBracket,
        YINI::TokenType::Identifier,
        YINI::TokenType::RightBracket,
        YINI::TokenType::LeftBracket,
        YINI::TokenType::Identifier,
        YINI::TokenType::RightBracket,
        YINI::TokenType::Eof,
    };

    std::vector<std::string> expected_literals = {
        "[", "Config", "]",
        "[", "#define", "]",
        "[", "#include", "]",
        "[", "#schema", "]",
        "",
    };

    for (size_t i = 0; i < expected_types.size(); ++i) {
        YINI::Token token = lexer.NextToken();
        ASSERT_EQ(token.type, expected_types[i]);
        ASSERT_EQ(token.literal, expected_literals[i]);
    }
}