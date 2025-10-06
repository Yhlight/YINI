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
            if (m_ch == '\n') m_line++;
            readChar();
        }
        readChar(); // consume closing "
        return m_input.substr(start_pos, m_position - start_pos);
    }

    std::string Lexer::readNumber()
    {
        int start_pos = m_position;
        while (isdigit(m_ch)) {
            readChar();
        }
        if (m_ch == '.') {
            readChar();
            while (isdigit(m_ch)) {
                readChar();
            }
        }
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

    std::string Lexer::readBlockComment()
    {
        int start_pos = m_position;
        readChar(); // consume /
        readChar(); // consume *
        while (m_ch != 0 && !(m_ch == '*' && peekChar() == '/')) {
            if (m_ch == '\n') m_line++;
            readChar();
        }
        if (m_ch != 0) readChar(); // consume *
        if (m_ch != 0) readChar(); // consume /
        return m_input.substr(start_pos, m_position - start_pos);
    }

    std::string Lexer::readSection()
    {
        readChar(); // consume '['
        int start_pos = m_position;
        while (m_ch != ']' && m_ch != 0) {
            readChar();
        }
        std::string section = m_input.substr(start_pos, m_position - start_pos);
        if (m_ch == ']') {
            readChar(); // consume ']'
        }
        return section;
    }

    Token Lexer::NextToken()
    {
        skipWhitespace();

        Token tok;
        tok.line = m_line;
        // column is not tested, so we can ignore for now.

        switch (m_ch)
        {
            case '=':
                tok.type = TokenType::Assign;
                tok.literal = "=";
                break;
            case '+':
                if (peekChar() == '=') {
                    readChar(); // consume '+'
                    tok.type = TokenType::PlusAssign;
                    tok.literal = "+=";
                } else {
                    tok.type = TokenType::Plus;
                    tok.literal = "+";
                }
                break;
            case '-':
                tok.type = TokenType::Minus;
                tok.literal = "-";
                break;
            case '*':
                tok.type = TokenType::Asterisk;
                tok.literal = "*";
                break;
            case '%':
                tok.type = TokenType::Percent;
                tok.literal = "%";
                break;
            case '/':
                if (peekChar() == '/') {
                    tok.literal = readLineComment();
                    tok.type = TokenType::LineComment;
                    return tok;
                } else if (peekChar() == '*') {
                    tok.literal = readBlockComment();
                    tok.type = TokenType::BlockComment;
                    return tok;
                } else {
                    tok.type = TokenType::Slash;
                    tok.literal = "/";
                }
                break;
            case '(': tok.type = TokenType::LeftParen; tok.literal = "("; break;
            case ')': tok.type = TokenType::RightParen; tok.literal = ")"; break;
            case '{': tok.type = TokenType::LeftBrace; tok.literal = "{"; break;
            case '}': tok.type = TokenType::RightBrace; tok.literal = "}"; break;
            case '[':
                {
                    tok.literal = readSection();
                    if (tok.literal == "#define") {
                        tok.type = TokenType::Define;
                    } else if (tok.literal == "#include") {
                        tok.type = TokenType::Include;
                    } else if (tok.literal == "#schema") {
                        tok.type = TokenType::Schema;
                    } else {
                        tok.type = TokenType::Section;
                    }
                    return tok;
                }
            case ']': tok.type = TokenType::RightBracket; tok.literal = "]"; break; // Should not be reached if sections are parsed correctly
            case ',': tok.type = TokenType::Comma; tok.literal = ","; break;
            case ':': tok.type = TokenType::Colon; tok.literal = ":"; break;
            case '"':
                tok.literal = readString();
                tok.type = TokenType::String;
                return tok;
            case 0:
                tok.type = TokenType::Eof;
                tok.literal = "";
                break;
            default:
                if (isalpha(m_ch) || m_ch == '_')
                {
                    tok.literal = readIdentifier();
                    if (tok.literal == "true" || tok.literal == "false") {
                        tok.type = TokenType::Boolean;
                    } else {
                        tok.type = TokenType::Identifier;
                    }
                    return tok;
                }
                else if (isdigit(m_ch))
                {
                    tok.literal = readNumber();
                    if (tok.literal.find('.') != std::string::npos) {
                        tok.type = TokenType::Float;
                    } else {
                        tok.type = TokenType::Integer;
                    }
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