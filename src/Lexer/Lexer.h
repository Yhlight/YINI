#pragma once

#include "Token.h"
#include <string>
#include <vector>

class Lexer
{
public:
    Lexer(const std::string& source);
    std::vector<Token> tokenize();

private:
    char peek();
    char advance();
    bool isAtEnd();
    void skipWhitespaceAndComments();
    Token makeToken(TokenType type);
    Token makeToken(TokenType type, const std::string& lexeme);
    Token scanIdentifier();
    Token scanString();
    Token scanNumber();
    Token scanHexLiteral();

    std::string source_code_;
    size_t start_ = 0;
    size_t current_ = 0;
    int line_ = 1;
    int column_ = 1;
};
