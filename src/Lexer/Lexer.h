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
    std::vector<Token> scan_tokens();

private:
    void scan_token();
    char advance();
    void add_token(TokenType type);
    void add_token(TokenType type, const std::variant<std::string, double>& literal);
    bool match(char expected);
    char peek();
    char peek_next();
    void string();
    void number();
    void identifier();
    bool is_at_end();

    std::string m_source;
    std::vector<Token> m_tokens;
    int m_start = 0;
    int m_current = 0;
    int m_line = 1;
};
}