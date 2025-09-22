#include <gtest/gtest.h>
#include "Lexer.h"
#include <vector>

TEST(LexerTest, BasicTokens)
{
    std::string input = R"yini(
[Section1]
key = "value"
number = 123
float_num = 3.14
is_true = true
is_false = false

// This is a comment
/* This is a
   multi-line comment */

[Section2]: Section1
+= item1
+= item2

[#define]
macro = some_value

[UI]
UIName = @macro
)yini";

    Yini::Lexer lexer(input);

    std::vector<Yini::Token> tokens;
    Yini::Token token;
    do {
        token = lexer.nextToken();
        tokens.push_back(token);
    } while (token.type != Yini::TokenType::EndOfFile);

    std::vector<Yini::TokenType> expected_types = {
        Yini::TokenType::LBracket, Yini::TokenType::Identifier, Yini::TokenType::RBracket,
        Yini::TokenType::Identifier, Yini::TokenType::Assign, Yini::TokenType::String,
        Yini::TokenType::Identifier, Yini::TokenType::Assign, Yini::TokenType::Integer,
        Yini::TokenType::Identifier, Yini::TokenType::Assign, Yini::TokenType::Float,
        Yini::TokenType::Identifier, Yini::TokenType::Assign, Yini::TokenType::Boolean,
        Yini::TokenType::Identifier, Yini::TokenType::Assign, Yini::TokenType::Boolean,
        Yini::TokenType::LBracket, Yini::TokenType::Identifier, Yini::TokenType::RBracket, Yini::TokenType::Colon, Yini::TokenType::Identifier,
        Yini::TokenType::PlusAssign, Yini::TokenType::Identifier,
        Yini::TokenType::PlusAssign, Yini::TokenType::Identifier,
        Yini::TokenType::LBracket, Yini::TokenType::Hash, Yini::TokenType::Identifier, Yini::TokenType::RBracket,
        Yini::TokenType::Identifier, Yini::TokenType::Assign, Yini::TokenType::Identifier,
        Yini::TokenType::LBracket, Yini::TokenType::Identifier, Yini::TokenType::RBracket,
        Yini::TokenType::Identifier, Yini::TokenType::Assign, Yini::TokenType::At, Yini::TokenType::Identifier,
        Yini::TokenType::EndOfFile
    };

    ASSERT_EQ(tokens.size(), expected_types.size());
    for (size_t i = 0; i < tokens.size(); ++i)
    {
        EXPECT_EQ(tokens[i].type, expected_types[i]);
    }
}
