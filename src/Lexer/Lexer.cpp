#include "Lexer.h"
#include <cctype>
#include <stdexcept>

namespace yini
{

Lexer::Lexer(const std::string& source)
    : source(source)
    , current(0)
    , line(1)
    , column(1)
    , token_start(0)
    , state(LexerState::INITIAL)
    , last_error("")
{
}

std::vector<Token> Lexer::tokenize()
{
    std::vector<Token> tokens;
    
    while (!isAtEnd())
    {
        Token token = nextToken();
        
        // Skip comments but keep newlines for parser
        if (token.type != TokenType::COMMENT)
        {
            tokens.push_back(token);
        }
        
        if (token.type == TokenType::ERROR)
        {
            break;
        }
    }
    
    // Add EOF token if not already present
    if (tokens.empty() || tokens.back().type != TokenType::END_OF_FILE)
    {
        tokens.push_back(makeToken(TokenType::END_OF_FILE));
    }
    
    return tokens;
}

Token Lexer::nextToken()
{
    skipWhitespace();
    
    token_start = current;
    
    if (isAtEnd())
    {
        return makeToken(TokenType::END_OF_FILE);
    }
    
    char c = advance();
    
    // Single character tokens
    switch (c)
    {
        case '\n':
            line++;
            column = 1;
            return makeToken(TokenType::NEWLINE);
        case '[': return makeToken(TokenType::LBRACKET);
        case ']': return makeToken(TokenType::RBRACKET);
        case '(': return makeToken(TokenType::LPAREN);
        case ')': return makeToken(TokenType::RPAREN);
        case '{': return makeToken(TokenType::LBRACE);
        case '}': return makeToken(TokenType::RBRACE);
        case ',': return makeToken(TokenType::COMMA);
        case ':': return makeToken(TokenType::COLON);
        
        // Handle dot - but only if not part of a number
        case '.':
            if (isDigit(peekNext()))
            {
                // This is a float starting with .
                return parseNumber();
            }
            return makeToken(TokenType::DOT);
        case '!': return makeToken(TokenType::EXCLAMATION);
        case '?': return makeToken(TokenType::QUESTION);
        case '~': return makeToken(TokenType::TILDE);
        case '*': return makeToken(TokenType::MULTIPLY);
        case '%': return makeToken(TokenType::MODULO);
        
        // Operators that might be multi-character
        case '+':
            if (match('='))
                return makeToken(TokenType::PLUS_EQUALS);
            return makeToken(TokenType::PLUS);
        
        case '-':
            return makeToken(TokenType::MINUS);
        
        case '=':
            return makeToken(TokenType::EQUALS);
        
        case '/':
            // Check for comments
            if (match('/'))
            {
                return handleCommentLine();
            }
            else if (match('*'))
            {
                return handleCommentBlock();
            }
            return makeToken(TokenType::DIVIDE);
        
        case '#':
            return parseColorOrHash();
        
        case '@':
            return parseAtSymbol();
        
        case '$':
            return parseDollarBrace();
        
        case '"':
            return parseString();
        
        default:
            // Numbers
            if (isDigit(c))
            {
                current--; // Back up to re-read the digit
                column--;
                return parseNumber();
            }
            
            // Identifiers and keywords
            if (isAlpha(c) || c == '_')
            {
                current--; // Back up to re-read the character
                column--;
                return parseIdentifier();
            }
            
            return makeError("Unexpected character");
    }
}

Token Lexer::peekToken()
{
    size_t save_current = current;
    size_t save_line = line;
    size_t save_column = column;
    
    Token token = nextToken();
    
    current = save_current;
    line = save_line;
    column = save_column;
    
    return token;
}

// State handlers

Token Lexer::handleCommentLine()
{
    // Skip until end of line
    while (!isAtEnd() && peek() != '\n')
    {
        advance();
    }
    
    return makeToken(TokenType::COMMENT);
}

Token Lexer::handleCommentBlock()
{
    // Skip until */
    while (!isAtEnd())
    {
        if (peek() == '*' && peekNext() == '/')
        {
            advance(); // *
            advance(); // /
            return makeToken(TokenType::COMMENT);
        }
        
        if (peek() == '\n')
        {
            line++;
            column = 0;
        }
        
        advance();
    }
    
    return makeError("Unterminated block comment");
}

Token Lexer::parseNumber()
{
    std::string num_str;
    bool has_dot = false;
    
    // Read integer part
    while (isDigit(peek()))
    {
        num_str += advance();
    }
    
    // Check for decimal point
    if (peek() == '.' && isDigit(peekNext()))
    {
        has_dot = true;
        num_str += advance(); // .
        
        while (isDigit(peek()))
        {
            num_str += advance();
        }
    }
    
    // Parse as float or integer
    if (has_dot)
    {
        return parseFloat(num_str);
    }
    else
    {
        return parseInteger(num_str);
    }
}

Token Lexer::parseInteger(const std::string& num_str)
{
    try
    {
        int64_t value = std::stoll(num_str);
        return makeToken(TokenType::INTEGER, value);
    }
    catch (const std::exception& e)
    {
        return makeError("Invalid integer: " + num_str);
    }
}

Token Lexer::parseFloat(const std::string& num_str)
{
    try
    {
        double value = std::stod(num_str);
        return makeToken(TokenType::FLOAT, value);
    }
    catch (const std::exception& e)
    {
        return makeError("Invalid float: " + num_str);
    }
}

Token Lexer::parseString()
{
    std::string str;
    
    while (!isAtEnd() && peek() != '"')
    {
        // Check string length limit
        if (str.length() >= MAX_STRING_LENGTH)
        {
            return makeError("String exceeds maximum length of " + 
                           std::to_string(MAX_STRING_LENGTH) + " bytes");
        }
        
        if (peek() == '\n')
        {
            line++;
            column = 0;
        }
        
        // Handle escape sequences
        if (peek() == '\\')
        {
            advance(); // backslash
            if (isAtEnd())
            {
                return makeError("Unterminated string");
            }
            
            char escaped = advance();
            switch (escaped)
            {
                case 'n': str += '\n'; break;
                case 't': str += '\t'; break;
                case 'r': str += '\r'; break;
                case '\\': str += '\\'; break;
                case '"': str += '"'; break;
                default: str += escaped; break;
            }
        }
        else
        {
            str += advance();
        }
    }
    
    if (isAtEnd())
    {
        return makeError("Unterminated string");
    }
    
    advance(); // Closing "
    
    return makeToken(TokenType::STRING, str);
}

Token Lexer::parseIdentifier()
{
    std::string identifier;
    
    while (isAlphaNumeric(peek()) || peek() == '_')
    {
        identifier += advance();
    }
    
    // Check for keywords
    TokenType type = identifierType(identifier);
    
    if (type == TokenType::BOOLEAN)
    {
        bool value = (identifier == "true");
        return makeToken(TokenType::BOOLEAN, value);
    }
    
    return makeToken(type, identifier);
}

TokenType Lexer::identifierType(const std::string& text)
{
    // Boolean literals
    if (text == "true" || text == "false")
    {
        return TokenType::BOOLEAN;
    }
    
    // Built-in type constructors
    if (text == "Color" || text == "color")
        return TokenType::COLOR;
    if (text == "Coord" || text == "coord")
        return TokenType::COORD;
    if (text == "Path" || text == "path")
        return TokenType::PATH;
    if (text == "List" || text == "list")
        return TokenType::LIST;
    if (text == "Array" || text == "array")
        return TokenType::ARRAY;
    if (text == "Dyna" || text == "dyna")
        return TokenType::DYNA;
    
    return TokenType::IDENTIFIER;
}

Token Lexer::parseColorOrHash()
{
    // Check if it's a hex color #RRGGBB
    // Must have at least 6 hex digits
    size_t hex_count = 0;
    size_t check_pos = current;
    
    while (check_pos < source.length() && hex_count < 8)
    {
        char c = source[check_pos];
        if (isDigit(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))
        {
            hex_count++;
            check_pos++;
        }
        else
        {
            break;
        }
    }
    
    // Only treat as color if we have exactly 6 or 8 hex digits
    if (hex_count == 6 || hex_count == 8)
    {
        std::string color = "#";
        
        // Read the hex digits
        for (size_t i = 0; i < hex_count; i++)
        {
            color += advance();
        }
        
        return makeToken(TokenType::COLOR, color);
    }
    
    // Otherwise it's just a hash symbol (for directives)
    return makeToken(TokenType::HASH);
}

Token Lexer::parseAtSymbol()
{
    if (match('{'))
    {
        return makeToken(TokenType::AT_LBRACE);
    }
    
    return makeToken(TokenType::AT);
}

Token Lexer::parseDollarBrace()
{
    if (match('{'))
    {
        return makeToken(TokenType::DOLLAR_LBRACE);
    }
    
    return makeError("Expected '{' after '$'");
}

// Helper functions

char Lexer::peek() const
{
    if (isAtEnd())
    {
        return '\0';
    }
    return source[current];
}

char Lexer::peekNext() const
{
    if (current + 1 >= source.length())
    {
        return '\0';
    }
    return source[current + 1];
}

char Lexer::advance()
{
    column++;
    return source[current++];
}

bool Lexer::isAtEnd() const
{
    return current >= source.length();
}

bool Lexer::match(char expected)
{
    if (isAtEnd())
    {
        return false;
    }
    if (source[current] != expected)
    {
        return false;
    }
    
    current++;
    column++;
    return true;
}

void Lexer::skipWhitespace()
{
    while (!isAtEnd())
    {
        char c = peek();
        
        switch (c)
        {
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
            default:
                return;
        }
    }
}

bool Lexer::isDigit(char c) const
{
    return c >= '0' && c <= '9';
}

bool Lexer::isAlpha(char c) const
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool Lexer::isAlphaNumeric(char c) const
{
    return isAlpha(c) || isDigit(c);
}

bool Lexer::isWhitespace(char c) const
{
    return c == ' ' || c == '\t' || c == '\r';
}

Token Lexer::makeToken(TokenType type)
{
    return Token(type, line, column - (current - token_start));
}

Token Lexer::makeToken(TokenType type, TokenValue value)
{
    return Token(type, value, line, column - (current - token_start));
}

Token Lexer::makeError(const std::string& message)
{
    last_error = "Error at line " + std::to_string(line) + ", column " + 
                 std::to_string(column) + ": " + message;
    return makeToken(TokenType::ERROR);
}

} // namespace yini
