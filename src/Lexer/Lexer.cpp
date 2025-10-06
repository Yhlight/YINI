#include "YINI/Lexer.h"
#include "YINI/Token.h"
#include <cctype>

namespace YINI
{
    Lexer::Lexer(const std::string& input)
        : m_input(input), m_position(0), m_readPosition(0), m_ch(0), m_line(1), m_column(0)
    {
        readChar();
    }

    void Lexer::readChar()
    {
        if (m_readPosition >= m_input.length())
        {
            m_ch = 0; // EOF
        }
        else
        {
            m_ch = m_input[m_readPosition];
        }
        m_position = m_readPosition;
        m_readPosition += 1;
        m_column += 1;
    }

    char Lexer::peekChar()
    {
        if (m_readPosition >= m_input.length()) {
            return 0;
        }
        return m_input[m_readPosition];
    }

    void Lexer::skipWhitespace()
    {
        while (m_ch == ' ' || m_ch == '\t' || m_ch == '\r' || m_ch == '\n')
        {
            if (m_ch == '\n')
            {
                m_line++;
                m_column = 0;
            }
            readChar();
        }
    }

    std::string Lexer::readIdentifier()
    {
        int start_pos = m_position;
        while (isalpha(m_ch) || isdigit(m_ch) || m_ch == '_') {
            readChar();
        }
        return m_input.substr(start_pos, m_position - start_pos);
    }

    std::string Lexer::readString()
    {
        int start_pos = m_position;
        readChar(); // consume opening "
        while (m_ch != '"' && m_ch != 0) {
            readChar();
        }
        readChar(); // consume closing "
        return m_input.substr(start_pos, m_position - start_pos);
    }

    std::string Lexer::readLineComment()
    {
        int start_pos = m_position;
        while (m_ch != '\n' && m_ch != 0) {
            readChar();
        }
        return m_input.substr(start_pos, m_position - start_pos);
    }

    Token Lexer::NextToken()
    {
        skipWhitespace();

        Token tok;
        tok.line = m_line;
        tok.column = m_column;

        switch (m_ch)
        {
            case '=':
                tok.type = TokenType::Assign;
                tok.literal = "=";
                break;
            case '[':
                tok.type = TokenType::LeftBracket;
                tok.literal = "[";
                break;
            case ']':
                tok.type = TokenType::RightBracket;
                tok.literal = "]";
                break;
            case '"':
                tok.type = TokenType::String;
                tok.literal = readString();
                return tok; // readString advances chars, so return early
            case '/':
                if (peekChar() == '/') {
                    tok.type = TokenType::LineComment;
                    tok.literal = readLineComment();
                    return tok; // readLineComment advances chars
                }
                // Fallthrough to illegal token for now
            case 0:
                tok.type = TokenType::Eof;
                tok.literal = "";
                break;
            default:
                if (isalpha(m_ch) || m_ch == '_')
                {
                    tok.literal = readIdentifier();
                    tok.type = TokenType::Identifier;
                    // No need to call readChar() here since readIdentifier() does it.
                    return tok;
                }
                else
                {
                    tok.type = TokenType::Illegal;
                    tok.literal = std::string(1, m_ch);
                }
                break;
        }

        readChar();
        return tok;
    }
}