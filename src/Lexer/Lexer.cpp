#include "Lexer.h"
#include "Core/YiniException.h"
#include <stdexcept>

namespace YINI
{
    const std::map<std::string, TokenType> Lexer::m_keywords = {
        {"true", TokenType::TRUE},
        {"false", TokenType::FALSE},
    };

    Lexer::Lexer(const std::string& source) : m_source(source) {}

    std::vector<Token> Lexer::scanTokens()
    {
        while (!isAtEnd())
        {
            m_start = m_current;
            m_start_column = m_column;
            scanToken();
        }

        m_tokens.push_back({TokenType::END_OF_FILE, "", std::any{}, m_line, m_column});
        return m_tokens;
    }

    void Lexer::scanToken()
    {
        char c = advance();
        switch (c)
        {
            case '(': addToken(TokenType::LEFT_PAREN); break;
            case ')': addToken(TokenType::RIGHT_PAREN); break;
            case '@': addToken(TokenType::AT); break;
            case '{': addToken(TokenType::LEFT_BRACE); break;
            case '}': addToken(TokenType::RIGHT_BRACE); break;
            case '[': addToken(TokenType::LEFT_BRACKET); break;
            case ']': addToken(TokenType::RIGHT_BRACKET); break;
            case '=': addToken(TokenType::EQUAL); break;
            case '+':
                addToken(match('=') ? TokenType::PLUS_EQUAL : TokenType::PLUS);
                break;
            case '-': addToken(TokenType::MINUS); break;
            case '*': addToken(TokenType::STAR); break;
            case '%': addToken(TokenType::PERCENT); break;
            case ',': addToken(TokenType::COMMA); break;
            case ':': addToken(TokenType::COLON); break;
            case '/':
                if (match('/'))
                {
                    // A comment goes until the end of the line.
                    while (peek() != '\n' && !isAtEnd()) advance();
                }
                else if (match('*'))
                {
                    blockComment();
                }
                else
                {
                    addToken(TokenType::SLASH);
                }
                break;
            case '"': string(); break;
            case ' ':
            case '\r':
            case '\t':
                // Ignore whitespace.
                break;
            case '\n':
                m_line++;
                m_column = 1;
                break;
            default:
                if (isdigit(c))
                {
                    number();
                }
                else if (isalpha(c) || c == '_' || c == '#')
                {
                    identifier();
                }
                else
                {
                    throw YiniException("Unexpected character.", m_line);
                }
                break;
        }
    }

    bool Lexer::match(char expected)
    {
        if (isAtEnd()) return false;
        if (m_source[m_current] != expected) return false;

        m_current++;
        return true;
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

    char Lexer::advance()
    {
        m_column++;
        return m_source[m_current++];
    }

    bool Lexer::isAtEnd()
    {
        return m_current >= m_source.length();
    }

    void Lexer::addToken(TokenType type)
    {
        addToken(type, std::any{});
    }

    void Lexer::addToken(TokenType type, const std::any& literal)
    {
        std::string text = m_source.substr(m_start, m_current - m_start);
        m_tokens.push_back({type, text, literal, m_line, m_start_column});
    }

    void Lexer::blockComment()
    {
        while (!(peek() == '*' && peekNext() == '/') && !isAtEnd())
        {
            if (peek() == '\n') m_line++;
            advance();
        }

        if (isAtEnd())
        {
            throw YiniException("Unterminated block comment.", m_line);
        }

        // Consume the */
        advance();
        advance();
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
            throw YiniException("Unterminated string.", m_line);
        }

        advance(); // The closing ".

        std::string value = m_source.substr(m_start + 1, m_current - m_start - 2);
        addToken(TokenType::STRING, value);
    }

    void Lexer::number()
    {
        while (isdigit(peek())) advance();

        if (peek() == '.' && isdigit(peekNext()))
        {
            advance(); // Consume the "."
            while (isdigit(peek())) advance();
        }

        addToken(TokenType::NUMBER, std::stod(m_source.substr(m_start, m_current - m_start)));
    }

    void Lexer::identifier()
    {
        while (isalnum(peek()) || peek() == '_' || peek() == '#') advance();

        std::string text = m_source.substr(m_start, m_current - m_start);
        TokenType type = TokenType::IDENTIFIER;
        auto it = m_keywords.find(text);
        if (it != m_keywords.end())
        {
            type = it->second;
        }

        if (type == TokenType::TRUE) {
            addToken(type, true);
        } else if (type == TokenType::FALSE) {
            addToken(type, false);
        } else {
            addToken(type, text);
        }
    }
}