#ifndef YINI_TOKEN_H
#define YINI_TOKEN_H

#include <string>

namespace YINI
{

enum class TokenType
{
    // Single-character tokens
    LEFT_BRACKET, RIGHT_BRACKET, // [ ]
    LEFT_BRACE, RIGHT_BRACE,   // { } for coordinates?
    COMMA,
    COLON,
    EQUAL,
    AT,                       // @
    HASH,                     // #

    // One or two character tokens
    PLUS_EQUAL,               // +=

    // Literals
    IDENTIFIER,
    STRING,
    INTEGER,
    FLOAT,
    BOOLEAN,

    // Comments
    COMMENT,

    // Other
    NEWLINE,
    END_OF_FILE,
    UNKNOWN
};

struct Token
{
    TokenType type;
    std::string literal;
    int line;
    int column;
};

} // namespace YINI

#endif // YINI_TOKEN_H
