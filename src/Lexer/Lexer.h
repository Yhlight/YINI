#pragma once

#include <string>
#include <vector>
#include "Token.h"

namespace Yini
{
class Lexer
{
public:
    Lexer(const std::string& source);
    std::vector<Token> scanTokens();

private:
    const std::string& source;
    std::vector<Token> tokens;
    int start = 0;
    int current = 0;
    int line = 1;

    bool isAtEnd();
    void scanToken();
    char advance();
    void addToken(TokenType type);
    void addToken(TokenType type, const std::variant<std::monostate, std::string, double, bool>& literal);
    char peek();
    char peekNext();
    bool match(char expected);

    void string();
    void number();
};
} // namespace Yini