#ifndef YINI_LEXER_LEXER_H
#define YINI_LEXER_LEXER_H

#include "Token.h"
#include "LexerState.h"
#include <string>
#include <vector>
#include <memory>

namespace yini
{

class Lexer
{
public:
    explicit Lexer(const std::string& source);
    
    // Tokenize the entire source
    std::vector<Token> tokenize();
    
    // Get next token
    Token nextToken();
    
    // Peek at current character without consuming
    char peek() const;
    
    // Peek ahead n characters
    char peekAhead(size_t n) const;
    
    // Consume and return current character
    char advance();
    
    // Put back last character (for state transitions)
    void unget();
    
    // Check if at end of source
    bool isAtEnd() const;
    
    // Get current position
    Position getPosition() const { return position_; }
    
    // Buffer management for building tokens
    void startBuffer() { buffer_.clear(); }
    void addToBuffer(char ch) { buffer_ += ch; }
    std::string getBuffer() const { return buffer_; }
    void clearBuffer() { buffer_.clear(); }
    
    // Emit a token
    void emitToken(TokenType type);
    void emitToken(TokenType type, const std::string& value);
    void emitToken(const Token& token);
    
    // Get emitted tokens
    const std::vector<Token>& getTokens() const { return tokens_; }
    
    // State management
    void setState(std::unique_ptr<LexerState> state);
    LexerState* getCurrentState() const { return current_state_.get(); }
    
    // Error handling
    void reportError(const std::string& message);
    bool hasErrors() const { return !errors_.empty(); }
    const std::vector<std::string>& getErrors() const { return errors_; }
    
    // Helper methods (public for state classes)
    bool isDigit(char ch) const;
    bool isAlpha(char ch) const;
    bool isAlphaNumeric(char ch) const;
    bool isWhitespace(char ch) const;
    TokenType identifyKeyword(const std::string& text) const;
    
private:
    std::string source_;
    size_t current_;
    Position position_;
    Position token_start_position_;
    
    std::string buffer_;
    std::vector<Token> tokens_;
    std::vector<std::string> errors_;
    
    std::unique_ptr<LexerState> current_state_;
};

} // namespace yini

#endif // YINI_LEXER_LEXER_H
