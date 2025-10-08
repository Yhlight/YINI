#pragma once

#include <string>
#include <variant>

namespace Yini
{
enum class TokenType
{
    // Single-character tokens
    LeftParen, RightParen,     // ( )
    LeftBrace, RightBrace,     // { }
    LeftBracket, RightBracket, // [ ]
    Comma, Dot, Minus, Plus,   // , . - +
    Semicolon, Slash, Star,    // ; / *
    Colon,                     // :

    // One or two character tokens
    Bang, BangEqual,           // ! !=
    Equal, EqualEqual,         // = ==
    Greater, GreaterEqual,     // > >=
    Less, LessEqual,           // < <=
    PlusEqual,                 // +=

    // Literals
    Identifier, String, Number,

    // Keywords
    True, False,
    Color, Coord, Path, List, Array,
    Dyna,

    // Others
    Comment,
    Eof
};

struct Token
{
    TokenType type;
    std::string lexeme;
    std::variant<std::monostate, std::string, double, bool> literal;
    int line;
};
} // namespace Yini