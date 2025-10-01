#pragma once

#include "Token.h"
#include <string>
#include <vector>

namespace YINI
{
    class Lexer
    {
    public:
        Lexer(const std::string& source);
        std::vector<Token> scanTokens();

    private:
        void scanToken();
        char advance();
        void addToken(TokenType type);
        void addToken(TokenType type, const std::any& literal);
        bool isAtEnd();
        char peek();
        void string();
        void identifier();

        std::string m_source;
        std::vector<Token> m_tokens;
        int m_start = 0;
        int m_current = 0;
        int m_line = 1;
    };
}