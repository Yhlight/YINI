#include "Lexer.h"
#include <cctype>
#include <iostream>

namespace YINI
{

Lexer::Lexer(const std::string& source) : m_source(source) {}

std::vector<Token> Lexer::tokenize()
{
    std::vector<Token> tokens;
    while (!isAtEnd())
    {
        m_start = m_current;
        char c = advance();
        switch (c)
        {
            case '[': tokens.push_back(makeToken(TokenType::LEFT_BRACKET)); break;
            case ']': tokens.push_back(makeToken(TokenType::RIGHT_BRACKET)); break;
            case '{': tokens.push_back(makeToken(TokenType::LEFT_BRACE)); break;
            case '}': tokens.push_back(makeToken(TokenType::RIGHT_BRACE)); break;
            case ',': tokens.push_back(makeToken(TokenType::COMMA)); break;
            case ':': tokens.push_back(makeToken(TokenType::COLON)); break;
            case '=': tokens.push_back(makeToken(TokenType::EQUAL)); break;
            case '@': tokens.push_back(makeToken(TokenType::AT)); break;
            case '#': tokens.push_back(makeToken(TokenType::HASH)); break;
            case '+':
                if (match('=')) tokens.push_back(makeToken(TokenType::PLUS_EQUAL));
                break;
            case '/':
                if (match('/')) {
                    while (peek() != '\n' && !isAtEnd()) advance();
                } else if (match('*')) {
                    while (!(peek() == '*' && peekNext() == '/') && !isAtEnd()) {
                        if (peek() == '\n') {
                            m_line++;
                            m_column = 0;
                        }
                        advance();
                    }
                    if (!isAtEnd()) {
                        advance(); // consume *
                        advance(); // consume /
                    }
                }
                break;
            case ' ':
            case '\r':
            case '\t':
                // Ignore whitespace
                break;
            case '\n':
                m_line++;
                m_column = 1;
                tokens.push_back(makeToken(TokenType::NEWLINE));
                break;
            case '"': tokens.push_back(string()); break;
            default:
                if (isdigit(c) || (c == '-' && isdigit(peek())))
                {
                    tokens.push_back(number());
                }
                else if (isalpha(c) || c == '_')
                {
                    tokens.push_back(identifier());
                }
                else
                {
                    tokens.push_back(makeToken(TokenType::UNKNOWN, std::string(1, c)));
                }
                break;
        }
    }
    tokens.push_back({TokenType::END_OF_FILE, "", m_line, m_column});
    return tokens;
}

Token Lexer::makeToken(TokenType type)
{
    return {type, m_source.substr(m_start, m_current - m_start), m_line, (int)(m_column - (m_current - m_start))};
}

Token Lexer::makeToken(TokenType type, const std::string& literal)
{
    return {type, literal, m_line, (int)(m_column - literal.length())};
}


char Lexer::advance()
{
    m_current++;
    char c = m_source[m_current - 1];
    if (c == '\n') {
        m_line++;
        m_column = 1;
    } else {
        m_column++;
    }
    return c;
}

bool Lexer::isAtEnd()
{
    return m_current >= m_source.length();
}

char Lexer::peek()
{
    if (isAtEnd()) return '\0';
    return m_source[m_current];
}

char Lexer::peekNext()
{
    if (m_current + 1 >= m_source.length()) return '\0';
    return m_source[m_current + 1];
}

bool Lexer::match(char expected)
{
    if (isAtEnd()) return false;
    if (m_source[m_current] != expected) return false;
    m_current++;
    m_column++;
    return true;
}

Token Lexer::string()
{
    int start_col = m_column;
    while (peek() != '"' && !isAtEnd())
    {
        advance();
    }

    if (isAtEnd())
    {
        return {TokenType::UNKNOWN, "Unterminated string.", m_line, start_col};
    }

    advance(); // The closing ".
    return {TokenType::STRING, m_source.substr(m_start + 1, m_current - m_start - 2), m_line, start_col};
}

Token Lexer::number()
{
    int start_col = m_column;
    bool isFloat = false;
    while (isdigit(peek())) advance();

    if (peek() == '.' && isdigit(peekNext()))
    {
        isFloat = true;
        advance(); // consume the "."
        while (isdigit(peek())) advance();
    }

    return {isFloat ? TokenType::FLOAT : TokenType::INTEGER, m_source.substr(m_start, m_current - m_start), m_line, start_col};
}

Token Lexer::identifier()
{
    int start_col = m_column;
    while (isalnum(peek()) || peek() == '_') advance();

    std::string text = m_source.substr(m_start, m_current - m_start);
    if (text == "true" || text == "false")
    {
        return {TokenType::BOOLEAN, text, m_line, start_col};
    }

    return {TokenType::IDENTIFIER, text, m_line, start_col};
}

} // namespace YINI
