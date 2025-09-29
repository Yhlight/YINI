#ifndef YINI_LEXER_HPP
#define YINI_LEXER_HPP

#include <string>
#include <vector>

namespace YINI
{
    enum class TokenType
    {
        Eof,
        Section,
        Key,
        Value,
        Equals,
        Comment,
        String,
        Number,
        Boolean,
        LeftBracket,
        RightBracket,
        LeftParen,
        RightParen,
        LeftBrace,
        RightBrace,
        Comma,
        Plus,
        Minus,
        Star,
        Slash,
        Percent,
        Colon,
        PlusEquals,
        At,
        Hash,
        HexColor,
        Identifier,
        Unknown
    };

    struct Token
    {
        TokenType type;
        std::string value;
        int line;
        int column;
    };

    class Lexer
    {
    public:
        Lexer(const std::string& input);
        Token getNextToken();

    private:
        void skipWhitespace();
        void skipComment();
        void skipBlockComment();
        Token parseString();
        Token parseNumber();
        Token parseIdentifier();

        std::string inputStr;
        size_t position;
        int line_num;
        int column_num;
    };
}

#endif // YINI_LEXER_HPP