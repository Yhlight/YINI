#include "Lexer.h"
#include <cctype>
#include <sstream>

namespace yini
{

Lexer::Lexer(const std::string& source)
    : source_(source), current_(0), position_(1, 1), token_start_position_(1, 1),
      current_state_(std::make_unique<DefaultState>())
{
}

std::vector<Token> Lexer::tokenize()
{
    while (!isAtEnd())
    {
        token_start_position_ = position_;
        char ch = advance();
        
        if (current_state_)
        {
            auto next_state = current_state_->process(*this, ch);
            if (next_state)
            {
                current_state_ = std::move(next_state);
            }
        }
    }
    
    // Process any remaining token in buffer at EOF
    if (current_state_ && !buffer_.empty())
    {
        token_start_position_ = position_;
        auto next_state = current_state_->process(*this, '\0');
        if (next_state)
        {
            current_state_ = std::move(next_state);
        }
    }
    
    // Emit EOF token
    emitToken(TokenType::EndOfFile);
    
    return tokens_;
}

Token Lexer::nextToken()
{
    if (tokens_.empty())
    {
        tokenize();
    }
    
    if (!tokens_.empty())
    {
        Token token = tokens_.front();
        tokens_.erase(tokens_.begin());
        return token;
    }
    
    return Token(TokenType::EndOfFile, "", position_);
}

char Lexer::peek() const
{
    if (isAtEnd())
        return '\0';
    return source_[current_];
}

char Lexer::peekAhead(size_t n) const
{
    if (current_ + n >= source_.length())
        return '\0';
    return source_[current_ + n];
}

char Lexer::advance()
{
    if (isAtEnd())
        return '\0';
    
    char ch = source_[current_++];
    
    if (ch == '\n')
    {
        position_.line++;
        position_.column = 1;
    }
    else
    {
        position_.column++;
    }
    
    return ch;
}

void Lexer::unget()
{
    if (current_ > 0)
    {
        current_--;
        if (position_.column > 1)
        {
            position_.column--;
        }
        else
        {
            // Moving back across a newline - this is complex, so we'll just decrement column
            // In practice, unget should only be called for single characters within a line
            if (position_.line > 1)
            {
                position_.line--;
            }
        }
    }
}

bool Lexer::isAtEnd() const
{
    return current_ >= source_.length();
}

void Lexer::emitToken(TokenType type)
{
    tokens_.emplace_back(type, buffer_, token_start_position_);
    clearBuffer();
}

void Lexer::emitToken(TokenType type, const std::string& value)
{
    tokens_.emplace_back(type, buffer_, value, token_start_position_);
    clearBuffer();
}

void Lexer::emitToken(const Token& token)
{
    tokens_.push_back(token);
}

void Lexer::setState(std::unique_ptr<LexerState> state)
{
    current_state_ = std::move(state);
}

void Lexer::reportError(const std::string& message)
{
    std::ostringstream oss;
    oss << "Lexer error at " << position_.line << ":" << position_.column 
        << " - " << message;
    errors_.push_back(oss.str());
}

bool Lexer::isDigit(char ch) const
{
    return std::isdigit(static_cast<unsigned char>(ch));
}

bool Lexer::isAlpha(char ch) const
{
    return std::isalpha(static_cast<unsigned char>(ch)) || ch == '_';
}

bool Lexer::isAlphaNumeric(char ch) const
{
    return isAlpha(ch) || isDigit(ch);
}

bool Lexer::isWhitespace(char ch) const
{
    return ch == ' ' || ch == '\t' || ch == '\r';
}

TokenType Lexer::identifyKeyword(const std::string& text) const
{
    if (text == "true") return TokenType::True;
    if (text == "false") return TokenType::False;
    if (text == "Dyna" || text == "dyna") return TokenType::Dyna;
    if (text == "Color" || text == "color") return TokenType::Color;
    if (text == "Coord" || text == "coord") return TokenType::Coord;
    if (text == "Path" || text == "path") return TokenType::Path;
    if (text == "List" || text == "list") return TokenType::List;
    if (text == "Array" || text == "array") return TokenType::Array;
    
    return TokenType::Identifier;
}

} // namespace yini
