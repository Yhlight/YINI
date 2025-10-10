#pragma once

#include <string>
#include <variant>

namespace YINI
{
enum class TokenType
{
    // Single-character tokens
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE, LEFT_BRACKET, RIGHT_BRACKET,
    COMMA, DOT, MINUS, PLUS, SLASH, STAR, PERCENT,
    COLON, SEMICOLON, EQUAL,

    // One or two character tokens
    PLUS_EQUAL, BANG, BANG_EQUAL,
    EQUAL_EQUAL, GREATER, GREATER_EQUAL,
    LESS, LESS_EQUAL,

    // Literals
    IDENTIFIER, STRING, NUMBER,

    // Keywords
    TRUE, FALSE,
    COLOR, COORD, PATH, LIST, ARRAY, DYNA,

    // Special
    HASH, AT, DOLLAR, TILDE, QUESTION,

    // Literals
    HEX_COLOR,

    // Comments
    SLASH_SLASH, SLASH_STAR, STAR_SLASH,

    // Others
    END_OF_FILE,
    UNKNOWN
};

struct Token
{
    TokenType type;
    std::string lexeme;
    std::variant<std::string, double> literal;
    int line;
    int column;
};
}