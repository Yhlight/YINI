#include "Lexer.h"
#include <map>

namespace YINI
{
    const std::map<std::string, TokenType> Lexer::m_keywords = {
        {"true",    TokenType::True},
        {"false",   TokenType::False},
        {"Coord",   TokenType::Coord},
        {"coord",   TokenType::Coord},
        {"Color",   TokenType::Color},
        {"color",   TokenType::Color},
        {"Path",    TokenType::Path},
        {"path",    TokenType::Path},
        {"Dyna",    TokenType::Dyna},
        {"dyna",    TokenType::Dyna}
    };

    Lexer::Lexer(const std::string& source) : m_source(source)
    {
    }

    bool Lexer::isAtEnd()
    {
        return m_current >= m_source.length();
    }

    char Lexer::advance()
    {
        return m_source[m_current++];
    }

    void Lexer::addToken(TokenType type)
    {
        addToken(type, {});
    }

    void Lexer::addToken(TokenType type, const std::variant<std::string, long long, double>& literal)
    {
        std::string text = m_source.substr(m_start, m_current - m_start);
        m_tokens.push_back({type, text, literal, m_line});
    }

    void Lexer::string()
    {
        while (peek() != '"' && !isAtEnd()) {
            if (peek() == '\n') m_line++;
            advance();
        }

        if (isAtEnd()) {
            // Unterminated string.
            // For now, we don't report an error, just don't create a token.
            // A more robust implementation would add an error token.
            return;
        }

        // The closing ".
        advance();

        // Trim the surrounding quotes.
        std::string value = m_source.substr(m_start + 1, m_current - m_start - 2);
        addToken(TokenType::String, value);
    }

    char Lexer::peekNext()
    {
        if (m_current + 1 >= m_source.length()) return '\0';
        return m_source[m_current + 1];
    }

    void Lexer::number()
    {
        while (isDigit(peek())) advance();

        // Look for a fractional part.
        if (peek() == '.' && isDigit(peekNext())) {
            // Consume the "."
            advance();

            while (isDigit(peek())) advance();
            addToken(TokenType::Float, std::stod(m_source.substr(m_start, m_current - m_start)));
            return;
        }

        addToken(TokenType::Integer, std::stoll(m_source.substr(m_start, m_current - m_start)));
    }

    bool Lexer::isAlpha(char c)
    {
        return (c >= 'a' && c <= 'z') ||
               (c >= 'A' && c <= 'Z') ||
                c == '_';
    }

    bool Lexer::isDigit(char c)
    {
        return c >= '0' && c <= '9';
    }

    bool Lexer::isAlphaNumeric(char c)
    {
        return isAlpha(c) || isDigit(c);
    }

    char Lexer::peek()
    {
        if (isAtEnd()) return '\0';
        return m_source[m_current];
    }

    void Lexer::identifier()
    {
        while (isAlphaNumeric(peek())) advance();

        std::string text = m_source.substr(m_start, m_current - m_start);
        TokenType type = TokenType::Identifier;
        auto it = m_keywords.find(text);
        if (it != m_keywords.end()) {
            type = it->second;
        }
        addToken(type);
    }

    void Lexer::scanToken()
    {
        char c = advance();
        switch (c)
        {
            case '[': addToken(TokenType::LBracket); break;
            case ']': addToken(TokenType::RBracket); break;
            case '{': addToken(TokenType::LBrace); break;
            case '}': addToken(TokenType::RBrace); break;
            case '(': addToken(TokenType::LParen); break;
            case ')': addToken(TokenType::RParen); break;
            case '=': addToken(TokenType::Equals); break;
            case ',': addToken(TokenType::Comma); break;
            case ':': addToken(TokenType::Colon); break;
            case '#': addToken(TokenType::Hash); break;
            case '@': addToken(TokenType::At); break;
            case '+':
                if (peek() == '=')
                {
                    advance();
                    addToken(TokenType::PlusEquals);
                } else {
                    addToken(TokenType::Plus);
                }
                break;
            case '-': addToken(TokenType::Minus); break;
            case '*': addToken(TokenType::Star); break;
            case '%': addToken(TokenType::Percent); break;
            case '/':
                if (peek() == '/') {
                    // A comment goes until the end of the line.
                    while (peek() != '\n' && !isAtEnd()) advance();
                } else if (peek() == '*') {
                    // A multi-line comment.
                    advance(); // consume the '*'
                    while (peek() != '*' && peekNext() != '/' && !isAtEnd()) {
                        if (peek() == '\n') m_line++;
                        advance();
                    }
                    if (!isAtEnd()) {
                        advance(); // consume the '*'
                        advance(); // consume the '/'
                    }
                } else {
                    addToken(TokenType::Slash);
                }
                break;
            // Ignore whitespace.
            case ' ':
            case '\r':
            case '\t':
                break;
            case '\n':
                m_line++;
                break;
            case '"': string(); break;
            default:
                if (isAlpha(c))
                {
                    identifier();
                }
                else if (isDigit(c))
                {
                    number();
                }
                // else: handle error, maybe add Unknown token
                break;
        }
    }

    std::vector<Token> Lexer::scanTokens()
    {
        while (!isAtEnd())
        {
            m_start = m_current;
            scanToken();
        }

        m_tokens.push_back({TokenType::EndOfFile, "", {}, m_line});
        return m_tokens;
    }
}