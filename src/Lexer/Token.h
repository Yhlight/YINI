#pragma once

#include <string>
#include <variant>

namespace YINI
{
    enum class TokenType
    {
        // Special Tokens
        EndOfFile,
        Invalid,

        // Literals
        Identifier,
        String,
        Integer,
        Float,
        Boolean,

        // Punctuation
        Assign,         // =
        PlusAssign,     // +=
        Comma,          // ,
        Colon,          // :
        LeftParen,      // (
        RightParen,     // )
        LeftBracket,    // [
        RightBracket,   // ]
        LeftBrace,      // {
        RightBrace,     // }

        // Keywords/Sections
        Section,        // [name]
        Define,         // [#define]
        Include,        // [#include]

        // Macro
        Macro,          // @name

        // Color
        Color,          // #RRGGBB, RGB(), rgb()

        // Comments
        LineComment,    // //
        BlockComment    // /* */
    };

    struct Token
    {
        TokenType type;
        std::string text;
        // std::variant<std::string, long long, double, bool> value;
    };
}
