#pragma once

#include "Token.h"
#include <string>
#include <vector>
#include <map>
#include <any>

namespace YINI
{
    class Lexer
    {
    public:
        Lexer(const std::string& source);
        std::vector<Token> scanTokens();

    private:
        void scanToken();
        bool match(char expected);
        char peek();
        char peekNext();
        char advance();
        bool isAtEnd();
        void addToken(TokenType type);
        void addToken(TokenType type, const std::any& literal);

        void blockComment();
        void string();
        void number();
        void identifier();

        std::string m_source;
        std::vector<Token> m_tokens;
        int m_start = 0;
        int m_current = 0;
        int m_line = 1;
        static const std::map<std::string, TokenType> m_keywords;
    };
}