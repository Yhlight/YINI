#ifndef YINI_TOKEN_H
#define YINI_TOKEN_H

#include <string>
#include <vector>

namespace Yini
{
    enum class TokenType
    {
        // Special tokens
        Illegal, // An illegal/unknown token
        Eof,     // End of file

        // Punctuation
        LBracket,  // [
        RBracket,  // ]
        LBrace,    // {
        RBrace,    // }
        LParen,    // (
        RParen,    // )
        Colon,     // :
        Comma,     // ,
        Assign,    // =
        PlusAssign,// +=
        At,        // @
        Hash,      // # (for colors and special sections)

        // Operators
        Plus,      // +
        Minus,     // -
        Star,      // *
        Slash,     // /
        Percent,   // %

        // Literals
        Identifier, // my_key, MySection
        Integer,    // 123
        Float,      // 3.14
        String,     // "hello"

        // Keywords (handled as Identifiers, but can be checked)
        // We will also check for case-insensitivity for these in the parser
        True,      // true
        False,     // false
        Coord,     // Coord, coord
        Color,     // Color, color
        Path,      // Path, path
        Dyna,      // Dyna, dyna
    };

    struct Token
    {
        TokenType type;
        std::string literal;
        int line = 0;
        int column = 0;

        Token(TokenType t = TokenType::Illegal, std::string lit = "", int l = 0, int c = 0)
            : type(t), literal(std::move(lit)), line(l), column(c) {}
    };
}

#endif //YINI_TOKEN_H
