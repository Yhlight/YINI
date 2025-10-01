#include "Lexer.h"
#include <stdexcept>

namespace YINI
{
    Lexer::Lexer(const std::string& source) : m_source(source)
    {
    }

    std::vector<Token> Lexer::scanTokens()
    {
        while (!isAtEnd())
        {
            m_start = m_current;
            scanToken();
        }

        m_tokens.push_back({TokenType::END_OF_FILE, "", std::any{}, m_line});
        return m_tokens;
    }

    void Lexer::scanToken()
    {
        char c = advance();
        switch (c)
        {
            case '[': addToken(TokenType::LEFT_BRACKET); break;
            case ']': addToken(TokenType::RIGHT_BRACKET); break;
            case '=': addToken(TokenType::EQUAL); break;
            case '"': string(); break;
            case ' ':
            case '\r':
            case '\t':
                // Ignore whitespace.
                break;
            case '\n':
                m_line++;
                break;
            default:
                if (isalpha(c))
                {
                    identifier();
                }
                else
                {
                    throw std::runtime_error("Unexpected character.");
                }
                break;
        }
    }

    char Lexer::advance()
    {
        return m_source[m_current++];
    }

    void Lexer::addToken(TokenType type)
    {
        addToken(type, std::any{});
    }

    void Lexer::addToken(TokenType type, const std::any& literal)
    {
        std::string text = m_source.substr(m_start, m_current - m_start);
        m_tokens.push_back({type, text, literal, m_line});
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

    void Lexer::string()
    {
        while (peek() != '"' && !isAtEnd())
        {
            if (peek() == '\n') m_line++;
            advance();
        }

        if (isAtEnd())
        {
            throw std::runtime_error("Unterminated string.");
        }

        // The closing ".
        advance();

        // Trim the surrounding quotes.
        std::string value = m_source.substr(m_start + 1, m_current - m_start - 2);
        addToken(TokenType::STRING, value);
    }

    void Lexer::identifier()
    {
        while (isalnum(peek())) advance();
        std::string text = m_source.substr(m_start, m_current - m_start);
        addToken(TokenType::IDENTIFIER, text);
    }
}