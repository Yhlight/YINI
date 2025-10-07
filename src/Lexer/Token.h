#ifndef YINI_TOKEN_H
#define YINI_TOKEN_H

#include <string>
#include <variant>

namespace YINI
{
    enum class TokenType
    {
        // Special
        EndOfFile,
        Error,

        // Literals
        Identifier,
        Integer,
        Float,
        String,
        EnvVar,

        // Punctuation
        LeftBracket,   // [
        RightBracket,  // ]
        LeftParen,     // (
        RightParen,    // )
        LeftBrace,     // {
        RightBrace,    // }
        Comma,         // ,
        Colon,         // :
        Dot,           // .
        Equals,        // =
        PlusEquals,    // +=
        At,            // @
        Hash,          // #

        // Operators
        Plus,          // +
        Minus,         // -
        Star,          // *
        Slash,         // /
        Percent,       // %

        // Keywords
        True,
        False,
    };

    struct Token
    {
        TokenType type;
        std::string text;
        int line;
        int column;
    };
}

#endif // YINI_TOKEN_H