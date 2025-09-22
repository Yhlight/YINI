#ifndef YINI_LEXER_H
#define YINI_LEXER_H

#include "Token.h"
#include <string>
#include <vector>

namespace YINI
{

class Lexer
{
public:
    Lexer(const std::string& source);
    std::vector<Token> tokenize();

private:
    std::string m_source;
    size_t m_current = 0;
    size_t m_start = 0;
    int m_line = 1;
    int m_column = 1;

    Token makeToken(TokenType type);
    Token makeToken(TokenType type, const std::string& literal);
    char advance();
    bool isAtEnd();
    char peek();
    char peekNext();
    bool match(char expected);

    void skipWhitespaceAndComments();
    Token string();
    Token number();
    Token identifier();
    Token specialSection();
};

} // namespace YINI

#endif // YINI_LEXER_H
