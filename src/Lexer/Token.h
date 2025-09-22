#pragma once

#include <string>
#include <utility>

namespace Yini
{
    enum class TokenType
    {
        // Special tokens
        Illegal,
        EndOfFile,

        // Literals
        Identifier, // key, section name
        Integer,    // 123
        Float,      // 3.14
        String,     // "hello"
        Boolean,    // true, false

        // Operators
        Assign,     // =
        PlusAssign, // +=
        At,         // @

        // Delimiters
        Comma,      // ,
        Colon,      // :
        LParen,     // (
        RParen,     // )
        LBracket,   // [
        RBracket,   // ]
        LBrace,     // {
        RBrace,     // }
        Hash,       // #

        // Comments
        Comment
    };

    struct Token
    {
        TokenType type;
        std::string literal;
        int line;
        int column;
    };
}
