#pragma once

#include "Token.h"
#include <string>

namespace Yini
{
    class Lexer
    {
    public:
        Lexer(const std::string& input);

        Token nextToken();

    private:
        void readChar();
        char peekChar() const;

        void skipWhitespace();
        void skipComment();

        Token readIdentifier();
        Token readNumber();
        Token readString();

        std::string m_input;
        size_t m_position;      // current position in input (points to current char)
        size_t m_readPosition;  // current reading position in input (after current char)
        char m_char;            // current char under examination

        int m_line;
        int m_column;
    };
}
