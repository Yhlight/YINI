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
        LeftBrace,
        RightBrace,
        Comma,
        Colon,
        PlusEquals,
        At,
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
        Token string();
        Token number();
        Token identifier();

        std::string m_input;
        size_t m_position;
        int m_line;
        int m_column;
    };
}

#endif // YINI_LEXER_HPP