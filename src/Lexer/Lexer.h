#ifndef YINI_LEXER_H
#define YINI_LEXER_H

#include "Token.h"
#include <string>

namespace YINI
{
    class Lexer
    {
    public:
        Lexer(const std::string& source);
        Token nextToken();
        Token peek();

    private:
        char currentChar();
        void advance();
        void skipWhitespace();
        Token identifier();
        Token number();
        Token string();
        Token envVar();

        std::string m_source;
        size_t m_position = 0;
        int m_line = 1;
        int m_column = 1;
    };
}

#endif // YINI_LEXER_H