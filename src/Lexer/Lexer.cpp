#include "Lexer.h"
#include <iostream>
#include <map>

namespace YINI
{

static std::map<std::string, TokenType> keywords = {
    {"true", TokenType::TRUE},
    {"false", TokenType::FALSE},
    {"color", TokenType::COLOR},
    {"Color", TokenType::COLOR},
    {"coord", TokenType::COORD},
    {"Coord", TokenType::COORD},
    {"path", TokenType::PATH},
    {"Path", TokenType::PATH},
    {"list", TokenType::LIST},
    {"List", TokenType::LIST},
    {"array", TokenType::ARRAY},
    {"Array", TokenType::ARRAY},
    {"dyna", TokenType::DYNA},
    {"Dyna", TokenType::DYNA}
};


Lexer::Lexer(const std::string& source) : m_source(source)
{
}

std::vector<Token> Lexer::scan_tokens()
{
    while (!is_at_end())
    {
        m_start = m_current;
        scan_token();
    }

    m_tokens.push_back({TokenType::END_OF_FILE, "", {}, m_line});
    return m_tokens;
}

void Lexer::scan_token()
{
    char c = advance();
    switch (c)
    {
    case '(': add_token(TokenType::LEFT_PAREN); break;
    case ')': add_token(TokenType::RIGHT_PAREN); break;
    case '{': add_token(TokenType::LEFT_BRACE); break;
    case '}': add_token(TokenType::RIGHT_BRACE); break;
    case '[': add_token(TokenType::LEFT_BRACKET); break;
    case ']': add_token(TokenType::RIGHT_BRACKET); break;
    case ',': add_token(TokenType::COMMA); break;
    case '.': add_token(TokenType::DOT); break;
    case '-': add_token(TokenType::MINUS); break;
    case '+':
        add_token(match('=') ? TokenType::PLUS_EQUAL : TokenType::PLUS);
        break;
    case '*': add_token(TokenType::STAR); break;
    case '%': add_token(TokenType::PERCENT); break;
    case ':': add_token(TokenType::COLON); break;
    case ';': add_token(TokenType::SEMICOLON); break;
    case '=':
        add_token(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
        break;
    case '!':
        add_token(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
        break;
    case '<':
        add_token(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
        break;
    case '>':
        add_token(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);
        break;
    case '/':
        if (match('/'))
        {
            // A comment goes until the end of the line.
            while (peek() != '\n' && !is_at_end()) advance();
        }
        else if (match('*'))
        {
            // A block comment goes until */
            while (!(peek() == '*' && peek_next() == '/') && !is_at_end())
            {
                if (peek() == '\n') m_line++;
                advance();
            }

            if (!is_at_end())
            {
                advance(); // consume *
                advance(); // consume /
            }
        }
        else
        {
            add_token(TokenType::SLASH);
        }
        break;
    case '#': add_token(TokenType::HASH); break;
    case '@': add_token(TokenType::AT); break;
    case '$': add_token(TokenType::DOLLAR); break;
    case '~': add_token(TokenType::TILDE); break;
    case '?': add_token(TokenType::QUESTION); break;

    case ' ':
    case '\r':
    case '\t':
        // Ignore whitespace.
        break;

    case '\n':
        m_line++;
        break;

    case '"': string(); break;

    default:
        if (isdigit(c))
        {
            number();
        }
        else if (isalpha(c) || c == '_')
        {
            identifier();
        }
        else
        {
            // std::cout << "Unexpected character: " << c << " at line " << m_line << std::endl;
            add_token(TokenType::UNKNOWN);
        }
        break;
    }
}

char Lexer::advance()
{
    return m_source[m_current++];
}

void Lexer::add_token(TokenType type)
{
    add_token(type, {});
}

void Lexer::add_token(TokenType type, const std::variant<std::string, double>& literal)
{
    std::string text = m_source.substr(m_start, m_current - m_start);
    m_tokens.push_back({type, text, literal, m_line});
}

bool Lexer::match(char expected)
{
    if (is_at_end()) return false;
    if (m_source[m_current] != expected) return false;

    m_current++;
    return true;
}

char Lexer::peek()
{
    if (is_at_end()) return '\0';
    return m_source[m_current];
}

char Lexer::peek_next()
{
    if (m_current + 1 >= m_source.length()) return '\0';
    return m_source[m_current + 1];
}

void Lexer::string()
{
    while (peek() != '"' && !is_at_end())
    {
        if (peek() == '\n') m_line++;
        advance();
    }

    if (is_at_end())
    {
        // Unterminated string.
        add_token(TokenType::UNKNOWN);
        return;
    }

    // The closing ".
    advance();

    // Trim the surrounding quotes.
    std::string value = m_source.substr(m_start + 1, m_current - m_start - 2);
    add_token(TokenType::STRING, value);
}

void Lexer::number()
{
    while (isdigit(peek())) advance();

    // Look for a fractional part.
    if (peek() == '.' && isdigit(peek_next()))
    {
        // Consume the "."
        advance();

        while (isdigit(peek())) advance();
    }

    add_token(TokenType::NUMBER, std::stod(m_source.substr(m_start, m_current - m_start)));
}

void Lexer::identifier()
{
    while (isalnum(peek()) || peek() == '_') advance();

    std::string text = m_source.substr(m_start, m_current - m_start);
    TokenType type = TokenType::IDENTIFIER;
    if (keywords.count(text))
    {
        type = keywords[text];
    }
    add_token(type);
}

bool Lexer::is_at_end()
{
    return m_current >= m_source.length();
}

}