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
    char peek();
    bool match(char expected);
};
} // namespace Yini