#pragma once

#include <string>
#include <variant>

namespace YINI
{
    enum class TokenType
    {
        // Special Tokens
        EndOfFile,
        Illegal,

        // Literals
        Identifier,
        Integer,
        Float,
        String,

        // Punctuation & Operators
        Assign,         // =
        PlusAssign,     // +=
        Plus,           // +
        Minus,          // -
        Asterisk,       // *
        Slash,          // /
        Percent,        // %
        LParen,         // (
        RParen,         // )
        LBracket,       // [
        RBracket,       // ]
        LBrace,         // {
        RBrace,         // }
        Comma,          // ,
        Colon,          // :
        At,             // @
        Hash,           // #

        // Keywords & Values
        True,
        False,
    };

    struct Token
    {
        TokenType type;
        std::string literal; // The raw text of the token
        // std::variant<int, double, std::string, bool> value; // The actual value, will be used later
        int line;
        int column;
    };
}