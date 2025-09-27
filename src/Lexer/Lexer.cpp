#include "Lexer/Lexer.h"
#include <cctype>

namespace YINI
{
    // Helper to check if a character can be part of an identifier
    bool is_letter(char ch) {
        return isalpha(ch) || ch == '_';
    }

    Lexer::Lexer(const std::string& input)
        : m_input(input), m_position(0), m_readPosition(0), m_char(0), m_line(1), m_column(0)
    {
        readChar(); // Initialize the first character
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

        if (m_char == '\n') {
            m_line++;
            m_column = 0;
        } else {
            m_column += 1;
        }
    }

    char Lexer::peekChar()
    {
        if (m_readPosition >= m_input.length()) {
            return 0;
        }
        return m_input[m_readPosition];
    }

    void Lexer::skip_whitespace()
    {
        while (m_char == ' ' || m_char == '\t' || m_char == '\n' || m_char == '\r')
        {
            readChar();
        }
    }

    void Lexer::skip_single_line_comment()
    {
        while (m_char != '\n' && m_char != 0)
        {
            readChar();
        }
    }

    void Lexer::skip_multi_line_comment()
    {
        // Assumes the initial '/*' has been consumed
        while (true) {
            if (m_char == 0) { // End of file in comment
                break;
            }
            if (m_char == '*' && peekChar() == '/') {
                readChar(); // Consume '*'
                readChar(); // Consume '/'
                break;
            }
            readChar();
        }
    }

    std::string Lexer::read_identifier()
    {
        size_t start_pos = m_position;
        while (is_letter(m_char))
        {
            readChar();
        }
        return m_input.substr(start_pos, m_position - start_pos);
    }


    Token Lexer::nextToken()
    {
        while (true) {
            skip_whitespace();

            if (m_char == '/')
            {
                if (peekChar() == '/')
                {
                    readChar(); // consume first '/'
                    readChar(); // consume second '/'
                    skip_single_line_comment();
                    continue; // restart loop to skip any subsequent whitespace
                }
                else if (peekChar() == '*')
                {
                    readChar(); // consume '/'
                    readChar(); // consume '*'
                    skip_multi_line_comment();
                    continue; // restart loop
                }
            }

            // If we're not in a comment or whitespace, we can break and process the token
            break;
        }

        Token token;
        token.line = m_line;
        token.column = m_column;

        if (is_letter(m_char))
        {
            token.literal = read_identifier();
            token.type = TokenType::Identifier;
            return token;
        }

        if (m_char == 0)
        {
            token.type = TokenType::EndOfFile;
            token.literal = "";
            return token;
        }

        // If we don't recognize the character, it's illegal
        token.type = TokenType::Illegal;
        token.literal = std::string(1, m_char);
        readChar();
        return token;
    }
}