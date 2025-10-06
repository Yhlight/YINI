#pragma once

#include "Token.h"
#include <string>

namespace YINI
{
    class Lexer
    {
    public:
        Lexer(const std::string& input);
        Token NextToken();

    private:
        void readChar();
        char peekChar();
        void skipWhitespace();
        std::string readIdentifier();
        std::string readString();
        std::string readLineComment();
        std::string readBlockComment();
        std::string readNumber();
        std::string readSection();

        std::string m_input;
        int m_position;      // current position in input (points to current char)
        int m_readPosition;  // current reading position in input (after current char)
        char m_ch;           // current char under examination
        int m_line;
        int m_column;
    };
}