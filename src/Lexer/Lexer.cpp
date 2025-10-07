#include "Lexer.h"
#include <cctype>

namespace YINI
{
    Lexer::Lexer(const std::string& source) : m_source(source) {}

    Token Lexer::nextToken()
    {
        skipWhitespace();

        if (m_position >= m_source.length())
        {
            return {TokenType::EndOfFile, "", m_line, m_column};
        }

        char ch = currentChar();

        if (isalpha(ch) || ch == '_')
        {
            return identifier();
        }

        if (isdigit(ch))
        {
            return number();
        }

        if (ch == '"')
        {
            return string();
        }

        if (ch == '$' && m_position + 1 < m_source.length() && m_source[m_position + 1] == '{')
        {
            return envVar();
        }

        switch (ch)
        {
            case '[': advance(); return {TokenType::LeftBracket, "[", m_line, m_column - 1};
            case ']': advance(); return {TokenType::RightBracket, "]", m_line, m_column - 1};
            case '(': advance(); return {TokenType::LeftParen, "(", m_line, m_column - 1};
            case ')': advance(); return {TokenType::RightParen, ")", m_line, m_column - 1};
            case '{': advance(); return {TokenType::LeftBrace, "{", m_line, m_column - 1};
            case '}': advance(); return {TokenType::RightBrace, "}", m_line, m_column - 1};
            case ',': advance(); return {TokenType::Comma, ",", m_line, m_column - 1};
            case ':': advance(); return {TokenType::Colon, ":", m_line, m_column - 1};
            case '.': advance(); return {TokenType::Dot, ".", m_line, m_column - 1};
            case '=': advance(); return {TokenType::Equals, "=", m_line, m_column - 1};
            case '+':
                if (m_position + 1 < m_source.length() && m_source[m_position + 1] == '=')
                {
                    advance();
                    advance();
                    return {TokenType::PlusEquals, "+=", m_line, m_column - 2};
                }
                advance();
                return {TokenType::Plus, "+", m_line, m_column - 1};
            case '-': advance(); return {TokenType::Minus, "-", m_line, m_column - 1};
            case '*': advance(); return {TokenType::Star, "*", m_line, m_column - 1};
            case '/': advance(); return {TokenType::Slash, "/", m_line, m_column - 1};
            case '%': advance(); return {TokenType::Percent, "%", m_line, m_column - 1};
            case '@': advance(); return {TokenType::At, "@", m_line, m_column - 1};
            case '#': advance(); return {TokenType::Hash, "#", m_line, m_column - 1};
        }


        return {TokenType::Error, std::string(1, ch), m_line, m_column};
    }

    Token Lexer::peek()
    {
        Lexer temp = *this;
        return temp.nextToken();
    }

    char Lexer::currentChar()
    {
        if (m_position >= m_source.length())
        {
            return '\0';
        }
        return m_source[m_position];
    }

    void Lexer::advance()
    {
        if (currentChar() == '\n')
        {
            m_line++;
            m_column = 1;
        }
        else
        {
            m_column++;
        }
        m_position++;
    }

    void Lexer::skipWhitespace()
    {
        while (true)
        {
            if (m_position >= m_source.length())
            {
                break;
            }

            if (isspace(currentChar()))
            {
                advance();
                continue;
            }

            // Single-line comment
            if (currentChar() == '/' && m_position + 1 < m_source.length() && m_source[m_position + 1] == '/')
            {
                while (m_position < m_source.length() && currentChar() != '\n')
                {
                    advance();
                }
                continue;
            }

            // Multi-line comment
            if (currentChar() == '/' && m_position + 1 < m_source.length() && m_source[m_position + 1] == '*')
            {
                advance(); // consume /
                advance(); // consume *
                while (m_position + 1 < m_source.length() && !(currentChar() == '*' && m_source[m_position + 1] == '/'))
                {
                    advance();
                }
                if (m_position + 1 < m_source.length())
                {
                    advance(); // consume *
                    advance(); // consume /
                }
                continue;
            }
            break;
        }
    }

    Token Lexer::identifier()
    {
        int start_col = m_column;
        size_t start_pos = m_position;
        while (m_position < m_source.length() && (isalnum(currentChar()) || currentChar() == '_'))
        {
            advance();
        }
        std::string text = m_source.substr(start_pos, m_position - start_pos);
        if (text == "true") return {TokenType::True, text, m_line, start_col};
        if (text == "false") return {TokenType::False, text, m_line, start_col};
        return {TokenType::Identifier, text, m_line, start_col};
    }

    Token Lexer::number()
    {
        int start_col = m_column;
        size_t start_pos = m_position;
        bool is_float = false;
        while (m_position < m_source.length() && isdigit(currentChar()))
        {
            advance();
        }
        if (m_position < m_source.length() && currentChar() == '.')
        {
            is_float = true;
            advance();
            while (m_position < m_source.length() && isdigit(currentChar()))
            {
                advance();
            }
        }
        std::string text = m_source.substr(start_pos, m_position - start_pos);
        return {is_float ? TokenType::Float : TokenType::Integer, text, m_line, start_col};
    }

    Token Lexer::string()
    {
        int start_col = m_column;
        size_t start_pos = m_position;
        advance(); // Consume opening quote
        while (m_position < m_source.length() && currentChar() != '"')
        {
            advance();
        }
        if (m_position >= m_source.length()) {
            return {TokenType::Error, "Unterminated string", m_line, start_col};
        }
        advance(); // Consume closing quote
        std::string text = m_source.substr(start_pos + 1, m_position - start_pos - 2);
        return {TokenType::String, text, m_line, start_col};
    }

    Token Lexer::envVar()
    {
        int start_col = m_column;
        advance(); // consume '$'
        advance(); // consume '{'

        size_t start_pos = m_position;
        while (m_position < m_source.length() && currentChar() != '}')
        {
            advance();
        }

        if (m_position >= m_source.length()) {
            return {TokenType::Error, "Unterminated environment variable", m_line, start_col};
        }

        std::string text = m_source.substr(start_pos, m_position - start_pos);
        advance(); // consume '}'

        return {TokenType::EnvVar, text, m_line, start_col};
    }
}