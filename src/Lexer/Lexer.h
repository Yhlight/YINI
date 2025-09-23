#ifndef YINI_LEXER_H
#define YINI_LEXER_H

#include "Token.h"
#include <string>

namespace Yini
{
    class Lexer
    {
    public:
        explicit Lexer(std::string input);

        Token nextToken();

    private:
        void readChar();
        char peekChar() const;

        void skipWhitespace();
        void skipComment();

        Token readIdentifier();
        Token readNumber();
        Token readString();

        static bool isLetter(char ch);
        static bool isDigit(char ch);

        std::string m_input;
        size_t m_position = 0;      // current position in input (points to current char)
        size_t m_readPosition = 0;  // current reading position in input (after current char)
        char m_char = 0;            // current char under examination
        int m_line = 1;
        int m_column = 0;
    };
}

#endif // YINI_LEXER_H
