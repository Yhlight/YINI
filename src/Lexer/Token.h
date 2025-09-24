#pragma once

#include <string>
#include <vector>

namespace YINI
{
    enum class TokenType
    {
        // Special Tokens
        ILLEGAL,
        END_OF_FILE,

        // Identifiers & Literals
        IDENTIFIER, // my_key, Config
        INTEGER,    // 123, -45
        FLOAT,      // 3.14
        STRING,     // "hello world"

        // Operators
        EQUAL,        // =
        PLUS_EQUAL,   // +=
        PLUS,         // +
        MINUS,        // -
        STAR,         // *
        SLASH,        // /
        PERCENT,      // %

        // Delimiters
        COMMA,        // ,
        COLON,        // :
        L_PAREN,      // (
        R_PAREN,      // )
        L_BRACKET,    // [
        R_BRACKET,    // ]
        L_BRACE,      // {
        R_BRACE,      // }

        // Keywords & Special Identifiers
        KEYWORD_TRUE,
        KEYWORD_FALSE,
        KEYWORD_COORD,
        KEYWORD_COLOR,
        KEYWORD_PATH,
        KEYWORD_DYNA,

        // Special Symbols
        AT,           // @
        HASH,         // # (For colors like #RRGGBB)
    };

    struct Token
    {
        TokenType type;
        std::string literal;
        int line = 0;
    };
}
