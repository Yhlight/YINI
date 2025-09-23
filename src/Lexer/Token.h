#pragma once

#include <string>

enum class TokenType
{
    // Single-character tokens
    L_BRACKET, R_BRACKET, // [ ]
    L_PAREN, R_PAREN,     // ( )
    L_BRACE, R_BRACE,     // { }
    COMMA,
    COLON,
    EQUAL,
    AT,
    HASH,

    // One or two character tokens
    PLUS_EQUAL, // +=

    // Literals
    IDENTIFIER,
    STRING,
    INTEGER,
    FLOAT,

    // Keywords (will be handled as identifiers by lexer, but defined here for parser)
    // TRUE, FALSE, COORD, COLOR, DEFINE, INCLUDE

    // Comments (usually skipped)
    COMMENT,

    // Special
    UNEXPECTED,
    END_OF_FILE
};

struct Token
{
    TokenType type;
    std::string lexeme;
    int line;
    int column;
};
