#include "Lexer.h"
#include <cctype>

namespace Yini
{
Lexer::Lexer(const std::string& source) : source(source) {}

std::vector<Token> Lexer::scanTokens()
{
    while (!isAtEnd())
    {
        start = current;
        scanToken();
    }

    tokens.push_back({TokenType::Eof, "", line});
    return tokens;
}

bool Lexer::isAtEnd()
{
    return current >= source.length();
}

char Lexer::advance()
{
    current++;
    return source[current - 1];
}

void Lexer::addToken(TokenType type)
{
    std::string text = source.substr(start, current - start);
    tokens.push_back({type, text, line});
}

char Lexer::peek()
{
    if (isAtEnd()) return '\0';
    return source[current];
}

bool Lexer::match(char expected) {
    if (isAtEnd()) return false;
    if (source[current] != expected) return false;

    current++;
    return true;
}

void Lexer::scanToken()
{
    char c = advance();
    switch (c)
    {
        case '[': addToken(TokenType::LeftBracket); break;
        case ']': addToken(TokenType::RightBracket); break;
        case ':': addToken(TokenType::Colon); break;
        case ',': addToken(TokenType::Comma); break;
        case '=': addToken(TokenType::Equal); break;
        case '+':
             if (match('=')) {
                 addToken(TokenType::PlusEqual);
             } else {
                 addToken(TokenType::Plus);
             }
             break;
        case ' ':
        case '\r':
        case '\t':
            // Ignore whitespace.
            break;
        case '\n':
            line++;
            break;
        default:
            if (isalpha(c) || c == '_') {
                while (isalnum(peek()) || peek() == '_') advance();
                addToken(TokenType::Identifier);
            }
            // Ignoring other characters for now
            break;
    }
}

} // namespace Yini