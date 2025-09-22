#include "Lexer.h"
#include <cctype>

namespace Yini
{
    namespace
    {
        // Redefine to not include digits
        bool isIdentifierStartChar(char c)
        {
            return std::isalpha(c) || c == '_';
        }

        bool isIdentifierChar(char c)
        {
            return std::isalnum(c) || c == '_';
        }
    }

    Lexer::Lexer(const std::string& input)
        : m_input(input), m_position(0), m_readPosition(0), m_char(0), m_line(1), m_column(0)
    {
        readChar();
    }

    void Lexer::readChar()
    {
        if (m_char == '\n')
        {
            m_line++;
            m_column = 0;
        }

        if (m_readPosition >= m_input.length())
        {
            m_char = 0; // EOF
        }
        else
        {
            m_char = m_input[m_readPosition];
        }
        m_position = m_readPosition;
        m_readPosition++;
        m_column++;
    }

    char Lexer::peekChar() const
    {
        if (m_readPosition >= m_input.length())
        {
            return 0; // EOF
        }
        return m_input[m_readPosition];
    }

    void Lexer::skipWhitespace()
    {
        while (std::isspace(m_char))
        {
            readChar();
        }
    }

    void Lexer::skipComment()
    {
        while (m_char == '/')
        {
            if (peekChar() == '/')
            {
                // Single line comment
                while (m_char != '\n' && m_char != 0)
                {
                    readChar();
                }
            }
            else if (peekChar() == '*')
            {
                // Multi-line comment
                readChar(); // consume '/'
                readChar(); // consume '*'
                while (m_char != 0)
                {
                    if (m_char == '*' && peekChar() == '/')
                    {
                        readChar(); // consume '*'
                        readChar(); // consume '/'
                        break;
                    }
                    readChar();
                }
            }
            else
            {
                break;
            }
            skipWhitespace();
        }
    }

    Token Lexer::readIdentifier()
    {
        int startLine = m_line;
        int startColumn = m_column;
        size_t startPos = m_position;
        while (isIdentifierChar(m_char))
        {
            readChar();
        }
        std::string literal = m_input.substr(startPos, m_position - startPos);

        if (literal == "true" || literal == "false")
        {
            return {TokenType::Boolean, literal, startLine, startColumn};
        }

        return {TokenType::Identifier, literal, startLine, startColumn};
    }

    Token Lexer::readNumber()
    {
        int startLine = m_line;
        int startColumn = m_column;
        size_t startPos = m_position;
        TokenType type = TokenType::Integer;
        while (std::isdigit(m_char))
        {
            readChar();
        }

        if (m_char == '.')
        {
            // Check if it is followed by a digit to be a float
            if(std::isdigit(peekChar()))
            {
                type = TokenType::Float;
                readChar();
                while (std::isdigit(m_char))
                {
                    readChar();
                }
            }
        }

        return {type, m_input.substr(startPos, m_position - startPos), startLine, startColumn};
    }

    Token Lexer::readString()
    {
        int startLine = m_line;
        int startColumn = m_column;
        readChar(); // consume opening "
        size_t startPos = m_position;
        while (m_char != '"' && m_char != 0)
        {
            readChar();
        }
        std::string literal = m_input.substr(startPos, m_position - startPos);
        readChar(); // consume closing "
        return {TokenType::String, literal, startLine, startColumn};
    }

    Token Lexer::nextToken()
    {
        skipWhitespace();
        skipComment();

        Token token;
        int startLine = m_line;
        int startColumn = m_column;

        switch (m_char)
        {
            case '=':
                token = {TokenType::Assign, "=", startLine, startColumn};
                readChar();
                break;
            case '+':
                if (peekChar() == '=')
                {
                    readChar();
                    token = {TokenType::PlusAssign, "+=", startLine, startColumn};
                    readChar();
                }
                else
                {
                    token = {TokenType::Illegal, "+", startLine, startColumn};
                    readChar();
                }
                break;
            case '@':
                token = {TokenType::At, "@", startLine, startColumn};
                readChar();
                break;
            case ',':
                token = {TokenType::Comma, ",", startLine, startColumn};
                readChar();
                break;
            case ':':
                token = {TokenType::Colon, ":", startLine, startColumn};
                readChar();
                break;
            case '(':
                token = {TokenType::LParen, "(", startLine, startColumn};
                readChar();
                break;
            case ')':
                token = {TokenType::RParen, ")", startLine, startColumn};
                readChar();
                break;
            case '[':
                token = {TokenType::LBracket, "[", startLine, startColumn};
                readChar();
                break;
            case ']':
                token = {TokenType::RBracket, "]", startLine, startColumn};
                readChar();
                break;
            case '{':
                token = {TokenType::LBrace, "{", startLine, startColumn};
                readChar();
                break;
            case '}':
                token = {TokenType::RBrace, "}", startLine, startColumn};
                readChar();
                break;
            case '#':
                token = {TokenType::Hash, "#", startLine, startColumn};
                readChar();
                break;
            case '"':
                return readString();
            case 0:
                token = {TokenType::EndOfFile, "", startLine, startColumn};
                break;
            default:
                if (std::isdigit(m_char))
                {
                    return readNumber();
                }
                else if (isIdentifierStartChar(m_char))
                {
                    return readIdentifier();
                }
                else
                {
                    token = {TokenType::Illegal, std::string(1, m_char), startLine, startColumn};
                    readChar();
                }
                break;
        }
        return token;
    }
}
