#ifndef YINI_TOKEN_H
#define YINI_TOKEN_H

#include <string>
#include <variant>
#include <vector>
#include <map>

namespace yini
{

// Token types enumeration
enum class TokenType
{
    // Basic tokens
    END_OF_FILE,
    NEWLINE,
    
    // Literals
    INTEGER,
    FLOAT,
    BOOLEAN,
    STRING,
    
    // Identifiers and keywords
    IDENTIFIER,
    
    // Section headers
    SECTION_START,      // [
    SECTION_END,        // ]
    COLON,             // :
    
    // Operators
    EQUALS,            // =
    PLUS_EQUALS,       // +=
    PLUS,              // +
    MINUS,             // -
    MULTIPLY,          // *
    DIVIDE,            // /
    MODULO,            // %
    
    // Delimiters
    COMMA,             // ,
    DOT,               // .
    LPAREN,            // (
    RPAREN,            // )
    LBRACKET,          // [
    RBRACKET,          // ]
    LBRACE,            // {
    RBRACE,            // }
    
    // Special
    AT,                // @ (for macro reference)
    DOLLAR_LBRACE,     // ${ (for environment variable)
    AT_LBRACE,         // @{ (for cross-section reference)
    HASH,              // # (for color or directives)
    EXCLAMATION,       // !
    QUESTION,          // ?
    TILDE,             // ~
    
    // Built-in types
    COLOR,
    COORD,
    PATH,
    LIST,
    ARRAY,
    MAP,
    DYNA,
    
    // Comments (usually skipped)
    COMMENT,
    
    // Error
    ERROR
};

// Token value types
using TokenValue = std::variant<
    std::monostate,    // No value
    int64_t,           // Integer
    double,            // Float
    bool,              // Boolean
    std::string        // String/Identifier
>;

// Token structure
struct Token
{
    TokenType type;
    TokenValue value;
    size_t line;
    size_t column;
    size_t length;
    
    Token(TokenType t = TokenType::ERROR, size_t l = 0, size_t c = 0, size_t len = 0)
        : type(t), value(std::monostate{}), line(l), column(c), length(len)
    {
    }
    
    Token(TokenType t, TokenValue v, size_t l, size_t c, size_t len = 0)
        : type(t), value(v), line(l), column(c), length(len)
    {
    }
    
    // Helper functions
    bool isType(TokenType t) const { return type == t; }
    
    std::string toString() const;
    
    // Type-safe value getters
    template<typename T>
    T getValue() const
    {
        return std::get<T>(value);
    }
    
    bool hasValue() const
    {
        return !std::holds_alternative<std::monostate>(value);
    }
};

} // namespace yini

#endif // YINI_TOKEN_H
