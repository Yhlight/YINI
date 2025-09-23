#include "Lexer.h"
#include <algorithm> // for std::transform
#include <cctype>    // for isspace

namespace Yini
{
    // Helper to convert string to lower case for keyword matching
    static std::string toLower(std::string s)
    {
        std::transform(s.begin(), s.end(), s.begin(),
            [](unsigned char c){ return std::tolower(c); });
        return s;
    }

    Lexer::Lexer(std::string input) : m_input(std::move(input))
    {
        readChar();
    }

    void Lexer::readChar()
    {
        if (m_readPosition >= m_input.length())
        {
            m_char = 0; // EOF
        }
        else
        {
            m_char = m_input[m_readPosition];
        }

        m_position = m_readPosition;
        m_readPosition += 1;

        if (m_char == '\n')
        {
            m_line++;
            m_column = 0;
        }
        else
        {
            m_column++;
        }
    }

    char Lexer::peekChar() const
    {
        if (m_readPosition >= m_input.length())
        {
            return 0;
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

    // This function assumes the initial characters ('//' or '/*') have been checked
    void Lexer::skipComment()
    {
        if (m_char == '/' && peekChar() == '/')
        {
            // Single line comment
            while (m_char != '\n' && m_char != 0)
            {
                readChar();
            }
        }
        else if (m_char == '/' && peekChar() == '*')
        {
            // Block comment
            readChar(); // consume '/'
            readChar(); // consume '*'
            while (true)
            {
                if (m_char == 0) break; // EOF
                if (m_char == '*' && peekChar() == '/')
                {
                    readChar(); // consume '*'
                    readChar(); // consume '/'
                    break;
                }
                readChar();
            }
        }
    }

    Token Lexer::readIdentifier()
    {
        int start_line = m_line;
        int start_column = m_column;
        size_t start_pos = m_position;
        while (isLetter(m_char) || isDigit(m_char))
        {
            readChar();
        }
        std::string literal = m_input.substr(start_pos, m_position - start_pos);

        // Check for keywords (case-insensitive)
        std::string lower_literal = toLower(literal);
        if (lower_literal == "true") return Token(TokenType::True, literal, start_line, start_column);
        if (lower_literal == "false") return Token(TokenType::False, literal, start_line, start_column);
        if (lower_literal == "coord") return Token(TokenType::Coord, literal, start_line, start_column);
        if (lower_literal == "color") return Token(TokenType::Color, literal, start_line, start_column);
        if (lower_literal == "path") return Token(TokenType::Path, literal, start_line, start_column);
        if (lower_literal == "dyna") return Token(TokenType::Dyna, literal, start_line, start_column);

        return Token(TokenType::Identifier, literal, start_line, start_column);
    }

    Token Lexer::readNumber()
    {
        int start_line = m_line;
        int start_column = m_column;
        size_t start_pos = m_position;
        bool is_float = false;
        while (isDigit(m_char) || m_char == '.')
        {
            if (m_char == '.')
            {
                if (peekChar() >= '0' && peekChar() <= '9')
                {
                    is_float = true;
                }
                else
                {
                    // This is a dot but not followed by a digit, so it's not part of the number.
                    break;
                }
            }
            readChar();
        }
        std::string literal = m_input.substr(start_pos, m_position - start_pos);
        TokenType type = is_float ? TokenType::Float : TokenType::Integer;
        return Token(type, literal, start_line, start_column);
    }

    Token Lexer::readString()
    {
        int start_line = m_line;
        int start_column = m_column;
        readChar(); // consume opening "
        size_t start_pos = m_position;
        while (m_char != '"' && m_char != 0)
        {
            // TODO: Handle escaped quotes \"
            readChar();
        }
        std::string literal = m_input.substr(start_pos, m_position - start_pos);
        readChar(); // consume closing "
        return Token(TokenType::String, literal, start_line, start_column);
    }

    bool Lexer::isLetter(char ch)
    {
        return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ch == '_';
    }

    bool Lexer::isDigit(char ch)
    {
        return '0' <= ch && ch <= '9';
    }

    Token Lexer::nextToken()
    {
        while (true)
        {
            skipWhitespace();

            if (m_char == '/')
            {
                if (peekChar() == '/' || peekChar() == '*')
                {
                    skipComment();
                    continue; // Restart loop to skip any subsequent whitespace/comments
                }
            }
            break; // No more whitespace or comments, break to tokenize
        }


        Token tok;
        tok.line = m_line;
        tok.column = m_column;

        switch (m_char)
        {
            case '=':
                tok = Token(TokenType::Assign, "=", m_line, m_column);
                break;
            case ':':
                tok = Token(TokenType::Colon, ":", m_line, m_column);
                break;
            case ',':
                tok = Token(TokenType::Comma, ",", m_line, m_column);
                break;
            case '(':
                tok = Token(TokenType::LParen, "(", m_line, m_column);
                break;
            case ')':
                tok = Token(TokenType::RParen, ")", m_line, m_column);
                break;
            case '{':
                tok = Token(TokenType::LBrace, "{", m_line, m_column);
                break;
            case '}':
                tok = Token(TokenType::RBrace, "}", m_line, m_column);
                break;
            case '[':
                tok = Token(TokenType::LBracket, "[", m_line, m_column);
                break;
            case ']':
                tok = Token(TokenType::RBracket, "]", m_line, m_column);
                break;
            case '@':
                tok = Token(TokenType::At, "@", m_line, m_column);
                break;
            case '#':
                tok = Token(TokenType::Hash, "#", m_line, m_column);
                break;
            case '+':
                if (peekChar() == '=')
                {
                    readChar();
                    tok = Token(TokenType::PlusAssign, "+=", m_line, m_column - 1);
                }
                else
                {
                    tok = Token(TokenType::Plus, "+", m_line, m_column);
                }
                break;
            case '-':
                tok = Token(TokenType::Minus, "-", m_line, m_column);
                break;
            case '*':
                tok = Token(TokenType::Star, "*", m_line, m_column);
                break;
            case '/':
                tok = Token(TokenType::Slash, "/", m_line, m_column);
                break;
            case '%':
                tok = Token(TokenType::Percent, "%", m_line, m_column);
                break;
            case '"':
                return readString();
            case 0:
                tok = Token(TokenType::Eof, "", m_line, m_column);
                break;
            default:
                if (isLetter(m_char))
                {
                    return readIdentifier();
                }
                else if (isDigit(m_char))
                {
                    return readNumber();
                }
                else
                {
                    tok = Token(TokenType::Illegal, std::string(1, m_char), m_line, m_column);
                }
                break;
        }

        readChar();
        return tok;
    }
}
