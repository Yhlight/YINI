#pragma once

#include "Core/YiniValue.h"
#include <string>
#include <vector>

namespace YINI
{
    enum class TokenType
    {
        // Single-character tokens.
        LEFT_BRACKET, RIGHT_BRACKET,
        LEFT_PAREN, RIGHT_PAREN,
        LEFT_BRACE, RIGHT_BRACE,
        EQUAL, SLASH, STAR, PLUS, MINUS, PERCENT, COMMA, COLON, AT,

        // Two-character tokens.
        PLUS_EQUAL, DOLLAR_LEFT_BRACE,

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
        YiniValue literal;
        int line;
        int column;
        std::string filepath;
    };
}