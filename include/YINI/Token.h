#pragma once

#include <string>
#include <variant>

namespace YINI
{
    enum class TokenType
    {
        // Special Tokens
        Illegal,
        Eof,

        // Literals
        Identifier,
        Integer,
        Float,
        String,
        Boolean,

        // Operators
        Assign,         // =
        PlusAssign,     // +=
        Plus,           // +
        Minus,          // -
        Asterisk,       // *
        Slash,          // /
        Percent,        // %


        // Delimiters
        LeftParen,      // (
        RightParen,     // )
        LeftBrace,      // {
        RightBrace,     // }
        LeftBracket,    // [
        RightBracket,   // ]
        Comma,          // ,
        Colon,          // :

        // Keywords & Sections
        Section,        // [SectionName]
        Define,         // [#define]
        Include,        // [#include]
        Schema,         // [#schema]

        // Comments
        LineComment,    // //
        BlockComment,   // /* */
    };

    struct Token
    {
        TokenType type;
        std::string literal;
        int line;
        int column;
    };
}