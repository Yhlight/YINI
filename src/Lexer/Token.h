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
        EQUAL, SLASH, STAR,

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