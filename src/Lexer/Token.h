#ifndef YINI_LEXER_TOKEN_H
#define YINI_LEXER_TOKEN_H

#include <string>
#include <optional>

namespace yini
{

// Token types
enum class TokenType
{
    // End of file
    EndOfFile,
    
    // Literals
    Integer,
    Float,
    Boolean,
    String,
    Identifier,
    
    // Keywords
    True,
    False,
    Dyna,
    Color,
    Coord,
    Path,
    List,
    Array,
    
    // Operators
    Plus,           // +
    Minus,          // -
    Star,           // *
    Slash,          // /
    Percent,        // %
    Assign,         // =
    PlusAssign,     // +=
    Colon,          // :
    Comma,          // ,
    Dot,            // .
    Hash,           // #
    At,             // @
    Dollar,         // $
    
    // Delimiters
    LeftBracket,    // [
    RightBracket,   // ]
    LeftParen,      // (
    RightParen,     // )
    LeftBrace,      // {
    RightBrace,     // }
    
    // Special
    NewLine,
    Comment,
    
    // Section markers
    Section,        // [Section]
    Define,         // [#define]
    Include,        // [#include]
    Schema,         // [#schema]
    
    // References
    MacroRef,       // @name
    EnvVar,         // ${name}
    CrossRef,       // @{Section.key}
    
    // Unknown
    Unknown
};

// Token position information
struct Position
{
    size_t line;
    size_t column;
    
    Position() : line(1), column(1) {}
    Position(size_t l, size_t c) : line(l), column(c) {}
};

// Token structure
class Token
{
public:
    Token();
    Token(TokenType type, const std::string& lexeme, Position pos);
    Token(TokenType type, const std::string& lexeme, const std::string& value, Position pos);
    
    TokenType getType() const { return type_; }
    const std::string& getLexeme() const { return lexeme_; }
    const std::string& getValue() const { return value_; }
    Position getPosition() const { return position_; }
    
    bool is(TokenType type) const { return type_ == type; }
    bool isOneOf(TokenType t1, TokenType t2) const { return type_ == t1 || type_ == t2; }
    
    std::string toString() const;
    
private:
    TokenType type_;
    std::string lexeme_;
    std::string value_;
    Position position_;
};

// Convert token type to string
std::string token_type_to_string(TokenType type);

} // namespace yini

#endif // YINI_LEXER_TOKEN_H
