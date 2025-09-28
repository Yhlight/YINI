#include "YINI/Lexer.hpp"
#include <cctype>

namespace YINI
{
    Lexer::Lexer(const std::string& input)
        : m_input(input), m_position(0), m_line(1), m_column(1) {}

    Token Lexer::getNextToken()
    {
        skipWhitespace();

        if (m_position >= m_input.length())
        {
            return {TokenType::Eof, "", m_line, m_column};
        }

        char current_char = m_input[m_position];

        if (current_char == '/' && m_position + 1 < m_input.length() && m_input[m_position + 1] == '/')
        {
            skipComment();
            return getNextToken(); // Get next token after comment
        }

        if (current_char == '[')
        {
            m_position++;
            m_column++;
            return {TokenType::LeftBracket, "[", m_line, m_column - 1};
        }

        if (current_char == ']')
        {
            m_position++;
            m_column++;
            return {TokenType::RightBracket, "]", m_line, m_column - 1};
        }

        if (current_char == ',')
        {
            m_position++;
            m_column++;
            return {TokenType::Comma, ",", m_line, m_column - 1};
        }

        if (current_char == ':')
        {
            m_position++;
            m_column++;
            return {TokenType::Colon, ":", m_line, m_column - 1};
        }

        if (current_char == '=')
        {
            m_position++;
            m_column++;
            return {TokenType::Equals, "=", m_line, m_column - 1};
        }

        if (current_char == '+' && m_position + 1 < m_input.length() && m_input[m_position + 1] == '=')
        {
            m_position += 2;
            m_column += 2;
            return {TokenType::PlusEquals, "+=", m_line, m_column - 2};
        }

        if (current_char == '"')
        {
            return string();
        }

        if (isdigit(current_char) || (current_char == '-' && m_position + 1 < m_input.length() && isdigit(m_input[m_position + 1])))
        {
            return number();
        }

        if (isalpha(current_char) || current_char == '_')
        {
            return identifier();
        }

        m_position++;
        m_column++;
        return {TokenType::Unknown, std::string(1, current_char), m_line, m_column - 1};
    }

    void Lexer::skipWhitespace()
    {
        while (m_position < m_input.length() && isspace(m_input[m_position]))
        {
            if (m_input[m_position] == '\n')
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
    }

    void Lexer::skipComment()
    {
        while (m_position < m_input.length() && m_input[m_position] != '\n')
        {
            m_position++;
            m_column++;
        }
    }

    Token Lexer::string()
    {
        std::string value;
        int start_col = m_column;
        m_position++; // Skip opening quote
        m_column++;

        while (m_position < m_input.length() && m_input[m_position] != '"')
        {
            value += m_input[m_position];
            m_position++;
            m_column++;
        }

        if (m_position < m_input.length())
        {
            m_position++; // Skip closing quote
            m_column++;
        }

        return {TokenType::String, value, m_line, start_col};
    }

    Token Lexer::number()
    {
        std::string value;
        int start_col = m_column;

        if(m_input[m_position] == '-')
        {
            value += m_input[m_position];
            m_position++;
            m_column++;
        }

        while (m_position < m_input.length() && (isdigit(m_input[m_position]) || m_input[m_position] == '.'))
        {
            value += m_input[m_position];
            m_position++;
            m_column++;
        }
        return {TokenType::Number, value, m_line, start_col};
    }

    Token Lexer::identifier()
    {
        std::string value;
        int start_col = m_column;
        while (m_position < m_input.length() && (isalnum(m_input[m_position]) || m_input[m_position] == '_'))
        {
            value += m_input[m_position];
            m_position++;
            m_column++;
        }

        if (value == "true" || value == "false")
        {
            return {TokenType::Boolean, value, m_line, start_col};
        }

        return {TokenType::Identifier, value, m_line, start_col};
    }
}