#pragma once

#include <string>
#include <vector>
#include <any>

namespace YINI
{
    enum class TokenType
    {
        // Single-character tokens.
        LEFT_BRACKET, RIGHT_BRACKET,
        LEFT_PAREN, RIGHT_PAREN,
        LEFT_BRACE, RIGHT_BRACE,
        EQUAL, SLASH, STAR, PLUS, MINUS, PERCENT, COMMA, COLON,

        // Literals.
        IDENTIFIER, STRING, NUMBER,

        // Keywords.
        TRUE, FALSE,

        // End of file.
        END_OF_FILE
    };

    struct Token
    {
        TokenType type;
        std::string lexeme;
        std::any literal;
        int line;
    };
}