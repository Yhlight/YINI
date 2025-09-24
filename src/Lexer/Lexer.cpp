#include "Lexer.h"
#include <cctype>
#include <unordered_map>
#include <string>

namespace YINI
{
    // Helper function to convert string to lower for case-insensitive keyword matching
    static std::string toLower(const std::string& str)
    {
        std::string lower_str = str;
        for (char& c : lower_str)
        {
            c = std::tolower(c);
        }
        return lower_str;
    }

    static const std::unordered_map<std::string, TokenType> keywords = {
        {"true", TokenType::KEYWORD_TRUE},
        {"false", TokenType::KEYWORD_FALSE},
        {"coord", TokenType::KEYWORD_COORD},
        {"color", TokenType::KEYWORD_COLOR},
        {"path", TokenType::KEYWORD_PATH},
        {"dyna", TokenType::KEYWORD_DYNA},
    };

    Lexer::Lexer(const std::string& input) : m_input(input)
    {
        readChar();
    }

    void Lexer::readChar()
    {
        if (m_read_position >= m_input.length())
        {
            m_char = 0; // EOF
        }
        else
        {
            m_char = m_input[m_read_position];
        }
        m_position = m_read_position;
        m_read_position += 1;
    }

    char Lexer::peekChar() const
    {
        if (m_read_position >= m_input.length())
        {
            return 0;
        }
        return m_input[m_read_position];
    }

    void Lexer::skipWhitespace()
    {
        while (m_char == ' ' || m_char == '\t' || m_char == '\r' || m_char == '\n')
        {
            if (m_char == '\n')
            {
                m_line++;
            }
            readChar();
        }
    }

    void Lexer::skipComment()
    {
        while (m_char != '\n' && m_char != 0)
        {
            readChar();
        }
    }

    void Lexer::skipMultilineComment()
    {
        readChar(); // Consume '*'
        while (true)
        {
            if (m_char == 0) // EOF
            {
                // Unterminated comment, let parser handle error
                break;
            }
            if (m_char == '*' && peekChar() == '/')
            {
                readChar(); // Consume '*'
                readChar(); // Consume '/'
                break;
            }
            if (m_char == '\n')
            {
                m_line++;
            }
            readChar();
        }
    }

    Token Lexer::readString()
    {
        size_t start_pos = m_position + 1;
        do
        {
            readChar();
            // Note: This simple version doesn't handle escaped quotes \"
        } while (m_char != '"' && m_char != 0);

        std::string literal = m_input.substr(start_pos, m_position - start_pos);
        return {TokenType::STRING, literal, m_line};
    }

    Token Lexer::readIdentifier()
    {
        size_t start_pos = m_position;
        while (std::isalpha(m_char) || m_char == '_') // Identifiers can start with letter or _
        {
            readChar();
        }

        std::string literal = m_input.substr(start_pos, m_position - start_pos);
        std::string lower_literal = toLower(literal);

        if (keywords.count(lower_literal))
        {
            return {keywords.at(lower_literal), literal, m_line};
        }

        return {TokenType::IDENTIFIER, literal, m_line};
    }

    Token Lexer::readNumber()
    {
        size_t start_pos = m_position;
        bool is_float = false;
        while (std::isdigit(m_char) || m_char == '.')
        {
            if (m_char == '.')
            {
                is_float = true;
            }
            readChar();
        }
        std::string literal = m_input.substr(start_pos, m_position - start_pos);
        return {is_float ? TokenType::FLOAT : TokenType::INTEGER, literal, m_line};
    }

    Token Lexer::nextToken()
    {
        Token token;

        skipWhitespace();

        switch (m_char)
        {
            case '=':
                token = {TokenType::EQUAL, "=", m_line};
                break;
            case '+':
                if (peekChar() == '=')
                {
                    readChar();
                    token = {TokenType::PLUS_EQUAL, "+=", m_line};
                }
                else
                {
                    token = {TokenType::PLUS, "+", m_line};
                }
                break;
            case '-':
                token = {TokenType::MINUS, "-", m_line};
                break;
            case '*':
                if (peekChar() == '/')
                {
                    // This is the end of a multiline comment, which should have been skipped.
                    // If we see it here, it's an error (e.g., */ without /*).
                    token = {TokenType::ILLEGAL, "*/", m_line};
                }
                else
                {
                    token = {TokenType::STAR, "*", m_line};
                }
                break;
            case '/':
                if (peekChar() == '/')
                {
                    skipComment();
                    return nextToken(); // Recursively call nextToken to get the token after the comment
                }
                else if (peekChar() == '*')
                {
                    readChar(); // Consume '/'
                    skipMultilineComment();
                    return nextToken(); // Continue tokenizing after comment
                }
                else
                {
                    token = {TokenType::SLASH, "/", m_line};
                }
                break;
            case '%':
                token = {TokenType::PERCENT, "%", m_line};
                break;
            case ',':
                token = {TokenType::COMMA, ",", m_line};
                break;
            case ':':
                token = {TokenType::COLON, ":", m_line};
                break;
            case '(':
                token = {TokenType::L_PAREN, "(", m_line};
                break;
            case ')':
                token = {TokenType::R_PAREN, ")", m_line};
                break;
            case '[':
                token = {TokenType::L_BRACKET, "[", m_line};
                break;
            case ']':
                token = {TokenType::R_BRACKET, "]", m_line};
                break;
            case '{':
                token = {TokenType::L_BRACE, "{", m_line};
                break;
            case '}':
                token = {TokenType::R_BRACE, "}", m_line};
                break;
            case '@':
                token = {TokenType::AT, "@", m_line};
                break;
            case '#':
                token = {TokenType::HASH, "#", m_line};
                break;
            case '"':
                token = readString();
                break;
            case 0:
                token = {TokenType::END_OF_FILE, "", m_line};
                break;
            default:
                if (std::isalpha(m_char) || m_char == '_')
                {
                    token = readIdentifier();
                    // readIdentifier advances m_char, so we don't call readChar() here
                    return token;
                }
                else if (std::isdigit(m_char))
                {
                    token = readNumber();
                    // readNumber advances m_char
                    return token;
                }
                else
                {
                    token = {TokenType::ILLEGAL, std::string(1, m_char), m_line};
                }
                break;
        }

        readChar();
        return token;
    }
}
