#ifndef YINI_LEXER_H
#define YINI_LEXER_H

#include "Token.h"
#include <string>
#include <vector>
#include <memory>
#include <optional>

namespace yini
{

// Lexer states for state machine
enum class LexerState
{
    INITIAL,
    IN_IDENTIFIER,
    IN_NUMBER,
    IN_STRING,
    IN_COMMENT_LINE,
    IN_COMMENT_BLOCK,
    IN_OPERATOR,
    IN_SPECIAL,
    ERROR_STATE
};

// Lexer class implementing state machine pattern
class Lexer
{
public:
    explicit Lexer(const std::string& source);
    ~Lexer() = default;
    
    // Main tokenization function
    std::vector<Token> tokenize();
    
    // Get next token
    Token nextToken();
    
    // Peek at next token without consuming
    Token peekToken();
    
    // Error reporting
    std::string getLastError() const { return last_error; }
    bool hasError() const { return !last_error.empty(); }
    
private:
    // State machine handlers
    Token handleInitialState();
    Token handleIdentifier();
    Token handleNumber();
    Token handleString();
    Token handleCommentLine();
    Token handleCommentBlock();
    Token handleOperator();
    Token handleSpecial();
    
    // Helper functions
    char peek() const;
    char peekNext() const;
    char advance();
    bool isAtEnd() const;
    bool match(char expected);
    void skipWhitespace();
    
    // Character classification
    bool isDigit(char c) const;
    bool isAlpha(char c) const;
    bool isAlphaNumeric(char c) const;
    bool isWhitespace(char c) const;
    
    // Token creation helpers
    Token makeToken(TokenType type);
    Token makeToken(TokenType type, TokenValue value);
    Token makeError(const std::string& message);
    
    // Number parsing
    Token parseNumber();
    Token parseInteger(const std::string& num_str);
    Token parseFloat(const std::string& num_str);
    
    // String parsing
    Token parseString();
    
    // Identifier/Keyword parsing
    Token parseIdentifier();
    TokenType identifierType(const std::string& text);
    
    // Special token parsing
    Token parseColorOrHash();
    Token parseAtSymbol();
    Token parseDollarBrace();
    
    // Source code and position tracking
    std::string source;
    size_t current;
    size_t line;
    size_t column;
    size_t token_start;
    
    // Current state
    LexerState state;
    
    // Resource limits
    static constexpr size_t MAX_STRING_LENGTH = 10 * 1024 * 1024; // 10MB
    static constexpr size_t MAX_IDENTIFIER_LENGTH = 1024;         // 1KB
    
    // Error tracking
    std::string last_error;
};

} // namespace yini

#endif // YINI_LEXER_H
