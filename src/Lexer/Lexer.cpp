#include "Lexer.h"
#include <cctype>

namespace YINI
{
    Lexer::Lexer(const std::string& input) : m_input(input)
    {
    }

    char Lexer::peek(size_t offset) const
    {
        if (m_pos + offset >= m_input.length())
        {
            return '\0';
        }
        return m_input[m_pos + offset];
    }

    char Lexer::advance()
    {
        if (m_pos < m_input.length())
        {
            m_pos++;
        }
        return m_input[m_pos - 1];
    }

    void Lexer::skipWhitespace()
    {
        while (isspace(peek()))
        {
            if (peek() == '\n')
            {
                m_at_start_of_line = true;
            }
            advance();
        }
    }

    void Lexer::skipComment()
    {
        if (peek() == '/' && peek(1) == '/')
        {
            advance();
            advance();
            while (peek() != '\n' && peek() != '\0')
            {
                advance();
            }
        }
        else if (peek() == '/' && peek(1) == '*')
        {
            advance();
            advance();
            while (peek() != '*' || peek(1) != '/')
            {
                if (peek() == '\0')
                {
                    // Unterminated block comment
                    break;
                }
                advance();
            }
            if (peek() == '*' && peek(1) == '/')
            {
                advance();
                advance();
            }
        }
    }

    Token Lexer::makeToken(TokenType type, const std::string& text)
    {
        return Token{type, text};
    }

    Token Lexer::identifier()
    {
        size_t start = m_pos;
        while (isalnum(peek()) || peek() == '_')
        {
            advance();
        }
        std::string text = m_input.substr(start, m_pos - start);
        if (text == "true" || text == "false")
        {
            return makeToken(TokenType::Boolean, text);
        }
        return makeToken(TokenType::Identifier, text);
    }

    Token Lexer::number()
    {
        size_t start = m_pos;
        bool is_float = false;
        while (isdigit(peek()) || (peek() == '.' && !is_float && isdigit(peek(1))))
        {
            if (peek() == '.')
            {
                is_float = true;
            }
            advance();
        }
        return makeToken(is_float ? TokenType::Float : TokenType::Integer, m_input.substr(start, m_pos - start));
    }

    Token Lexer::string()
    {
        advance(); // Consume the opening quote
        size_t start = m_pos;
        while (peek() != '"' && peek() != '\0')
        {
            advance();
        }
        std::string value = m_input.substr(start, m_pos - start);
        if (peek() == '"')
        {
            advance(); // Consume the closing quote
        }
        return makeToken(TokenType::String, value);
    }

    Token Lexer::section()
    {
        advance(); // Consume the opening bracket
        size_t start = m_pos;
        while (peek() != ']' && peek() != '\0')
        {
            advance();
        }
        std::string value = m_input.substr(start, m_pos - start);
        if (peek() == ']')
        {
            advance(); // Consume the closing bracket
        }

        if (value == "#define")
        {
            return makeToken(TokenType::Define, value);
        }
        else if (value == "#include")
        {
            return makeToken(TokenType::Include, value);
        }

        return makeToken(TokenType::Section, value);
    }

    Token Lexer::color()
    {
        advance(); // consume #
        size_t start = m_pos;
        for(int i=0; i<6; ++i)
        {
            if(!isxdigit(peek()))
            {
                return makeToken(TokenType::Invalid, "#" + m_input.substr(start, m_pos-start));
            }
            advance();
        }
        return makeToken(TokenType::Color, m_input.substr(start, m_pos-start));
    }


    Token Lexer::getNextToken()
    {
        skipWhitespace();
        if (peek() == '/' && (peek(1) == '/' || peek(1) == '*'))
        {
            skipComment();
        }
        skipWhitespace();

        if (peek() == '\0')
        {
            return makeToken(TokenType::EndOfFile, "");
        }

        char current_char = peek();
        bool is_start = m_at_start_of_line;
        m_at_start_of_line = false; // Reset for the next token

        if (is_start && current_char == '[')
        {
            return section();
        }

        if (isalpha(current_char) || current_char == '_')
        {
            return identifier();
        }

        if (isdigit(current_char))
        {
            return number();
        }

        if (current_char == '"')
        {
            return string();
        }

        if (current_char == '@')
        {
            advance(); // consume @
            size_t start = m_pos;
            while(isalnum(peek()) || peek() == '_')
            {
                advance();
            }
            return makeToken(TokenType::Macro, m_input.substr(start, m_pos-start));
        }

        switch (current_char)
        {
            case '=':
                advance();
                return makeToken(TokenType::Assign, "=");
            case '+':
                advance();
                if (peek() == '=')
                {
                    advance();
                    return makeToken(TokenType::PlusAssign, "+=");
                }
                break; // Fallthrough to invalid
            case ',':
                advance();
                return makeToken(TokenType::Comma, ",");
            case ':':
                advance();
                return makeToken(TokenType::Colon, ":");
            case '(':
                advance();
                return makeToken(TokenType::LeftParen, "(");
            case ')':
                advance();
                return makeToken(TokenType::RightParen, ")");
            case '[':
                advance();
                return makeToken(TokenType::LeftBracket, "[");
            case ']':
                advance();
                return makeToken(TokenType::RightBracket, "]");
            case '{':
                advance();
                return makeToken(TokenType::LeftBrace, "{");
            case '}':
                advance();
                return makeToken(TokenType::RightBrace, "}");
            case '#':
                return color();
        }

        advance();
        return makeToken(TokenType::Invalid, std::string(1, current_char));
    }
}
