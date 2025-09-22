#pragma once

#include "Token.h"
#include <string>
#include <vector>

namespace YINI
{
    class Lexer
    {
    public:
        Lexer(const std::string& input);

        Token getNextToken();

    private:
        char peek(size_t offset = 0) const;
        char advance();
        void skipWhitespace();
        void skipComment();

        Token makeToken(TokenType type, const std::string& text);
        Token identifier();
        Token number();
        Token string();
        Token section();
        Token color();

        std::string m_input;
        size_t m_pos = 0;
        bool m_at_start_of_line = true;
    };
}
