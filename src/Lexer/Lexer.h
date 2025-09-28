#pragma once

#include "Token.h"
#include <string>
#include <vector>
#include <map>

namespace YINI
{
    class Lexer
    {
    public:
        Lexer(const std::string& source);
        std::vector<Token> scanTokens();

    private:
        const std::string& m_source;
        std::vector<Token> m_tokens;
        int m_start = 0;
        int m_current = 0;
        int m_line = 1;
        static const std::map<std::string, TokenType> m_keywords;

        void scanToken();
        bool isAtEnd();
        char advance();
        char peek();
        char peekNext();
        void addToken(TokenType type);
        void addToken(TokenType type, const std::variant<std::string, long long, double>& literal);
        void identifier();
        void string();
        void number();

        bool isAlpha(char c);
        bool isDigit(char c);
        bool isAlphaNumeric(char c);
    };
}