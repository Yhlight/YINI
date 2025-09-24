#pragma once

#include "Token.h"
#include <string>

namespace YINI
{
    class Lexer
    {
    public:
        Lexer(const std::string& input);

        Token nextToken();

    private:
        void readChar();
        char peekChar() const;

        Token readString();
        Token readIdentifier();
        Token readNumber();
        void skipWhitespace();
        void skipComment();
        void skipMultilineComment();

        std::string m_input;
        size_t m_position = 0;      // current position in input (points to current char)
        size_t m_read_position = 0; // current reading position in input (after current char)
        char m_char = 0;            // current char under examination
        int m_line = 1;             // current line number for error reporting
    };
}
