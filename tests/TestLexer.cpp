#include "gtest/gtest.h"
#include "YINI/Lexer.h"
#include "YINI/Token.h"
#include <vector>

TEST(LexerTest, BasicTokens) {
    std::string input = R"([Config]
// This is a comment
key = "value"
)";

    YINI::Lexer lexer(input);

    std::vector<YINI::TokenType> expected_types = {
        YINI::TokenType::LeftBracket,
        YINI::TokenType::Identifier,
        YINI::TokenType::RightBracket,
        YINI::TokenType::LineComment,
        YINI::TokenType::Identifier,
        YINI::TokenType::Assign,
        YINI::TokenType::String,
        YINI::TokenType::Eof,
    };

    std::vector<std::string> expected_literals = {
        "[",
        "Config",
        "]",
        "// This is a comment",
        "key",
        "=",
        "\"value\"",
        "",
    };

    for (size_t i = 0; i < expected_types.size(); ++i)
    {
        YINI::Token token = lexer.NextToken();
        ASSERT_EQ(token.type, expected_types[i]);
        ASSERT_EQ(token.literal, expected_literals[i]);
    }
}