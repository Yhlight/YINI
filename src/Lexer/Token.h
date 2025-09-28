#pragma once

#include <string>
#include <variant>

namespace YINI
{
    enum class TokenType
    {
        // Punctuation
        LBracket,      // [
        RBracket,      // ]
        LBrace,        // {
        RBrace,        // }
        LParen,        // (
        RParen,        // )
        Equals,        // =
        PlusEquals,    // +=
        Comma,         // ,
        Colon,         // :
        Hash,          // #
        At,            // @
        Slash,         // /
        Plus,          // +
        Minus,         // -
        Star,          // *
        Percent,       // %

        // Literals
        Identifier,
        String,
        Integer,
        Float,

        // Keywords
        True,
        False,
        Coord,
        Color,
        Path,
        Dyna,

        // Other
        Comment,
        EndOfFile,
        Unknown
    };

    struct Token
    {
        TokenType type;
        std::string lexeme;
        std::variant<std::string, long long, double> literal;
        int line;
    };
}