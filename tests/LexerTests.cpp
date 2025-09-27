#include <gtest/gtest.h>
#include "Lexer/Lexer.h"
#include "Lexer/Token.h"
#include <vector>

TEST(LexerTest, SkipsCommentsAndFindsNextToken)
{
    std::string input = R"(
        // This is a single-line comment.
        /* This is a
           multi-line comment. */

        ident // Another comment
    )";

    YINI::Lexer lexer(input);

    std::vector<YINI::TokenType> expected_tokens = {
        YINI::TokenType::Identifier,
        YINI::TokenType::EndOfFile
    };

    for(const auto& expected_type : expected_tokens) {
        YINI::Token token = lexer.nextToken();
        EXPECT_EQ(token.type, expected_type);
    }
}