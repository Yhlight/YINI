#include "Lexer/Lexer.h"
#include <cctype>

namespace YINI
{
    // Helper to check if a character can be part of an identifier
    bool is_letter(char ch) {
        return isalpha(ch) || ch == '_';
    }

    // Helper to check if a character is a digit
    bool is_digit(char ch) {
        return isdigit(ch);
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

    std::string Lexer::read_string()
    {
        size_t start_pos = m_position;
        readChar(); // Consume the opening quote
        while (m_char != '"' && m_char != 0)
        {
            readChar();
        }
        std::string literal = m_input.substr(start_pos + 1, m_position - start_pos - 1);
        readChar(); // Consume the closing quote
        return literal;
    }

    Token Lexer::read_number()
    {
        Token token;
        token.line = m_line;
        token.column = m_column;
        token.type = TokenType::Integer; // Assume integer by default

        size_t start_pos = m_position;
        while (is_digit(m_char)) {
            readChar();
        }

        if (m_char == '.' && is_digit(peekChar())) {
            token.type = TokenType::Float;
            readChar(); // consume the '.'
            while (is_digit(m_char)) {
                readChar();
            }
        }

        token.literal = m_input.substr(start_pos, m_position - start_pos);
        return token;
    }

    Token Lexer::nextToken()
    {
        // This loop handles skipping whitespace and comments.
        // It will continue until a character is found that is part of a token.
        while (true) {
            skip_whitespace();

            if (m_char == '/')
            {
                if (peekChar() == '/')
                {
                    readChar(); // consume first '/'
                    readChar(); // consume second '/'
                    skip_single_line_comment();
                    continue; // restart loop to find next token
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

        // Handle multi-character tokens first
        if (is_letter(m_char))
        {
            token.literal = read_identifier();
            // Check if the identifier is a keyword
            if (token.literal == "true") {
                token.type = TokenType::True;
            } else if (token.literal == "false") {
                token.type = TokenType::False;
            } else {
                token.type = TokenType::Identifier;
            }
            return token; // read_identifier already advanced the char, so we return early.
        }

        if (m_char == '"')
        {
            token.literal = read_string();
            token.type = TokenType::String;
            return token; // read_string already advanced the char, so we return early.
        }

        if (is_digit(m_char)) {
            return read_number();
        }

        // Handle single-character tokens
        switch(m_char)
        {
            case '=':
                token.type = TokenType::Assign;
                token.literal = "=";
                break;
            case '+':
                if (peekChar() == '=') {
                    char ch = m_char;
                    readChar();
                    token.type = TokenType::PlusAssign;
                    token.literal = std::string(1, ch) + std::string(1, m_char);
                } else {
                    token.type = TokenType::Plus;
                    token.literal = "+";
                }
                break;
            case '-':
                token.type = TokenType::Minus;
                token.literal = "-";
                break;
            case '*':
                token.type = TokenType::Asterisk;
                token.literal = "*";
                break;
            case '/':
                token.type = TokenType::Slash;
                token.literal = "/";
                break;
            case '%':
                token.type = TokenType::Percent;
                token.literal = "%";
                break;
            case '(':
                token.type = TokenType::LParen;
                token.literal = "(";
                break;
            case ')':
                token.type = TokenType::RParen;
                token.literal = ")";
                break;
            case '[':
                token.type = TokenType::LBracket;
                token.literal = "[";
                break;
            case ']':
                token.type = TokenType::RBracket;
                token.literal = "]";
                break;
            case 0:
                token.type = TokenType::EndOfFile;
                token.literal = "";
                break;
            default:
                token.type = TokenType::Illegal;
                token.literal = std::string(1, m_char);
                break;
        }

        readChar(); // Advance to the next character for single-char tokens
        return token;
    }
}