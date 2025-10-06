#include "Token.h"
#include <sstream>

namespace yini
{

Token::Token() 
    : type_(TokenType::Unknown), lexeme_(""), value_(""), position_(Position()) {}

Token::Token(TokenType type, const std::string& lexeme, Position pos)
    : type_(type), lexeme_(lexeme), value_(lexeme), position_(pos) {}

Token::Token(TokenType type, const std::string& lexeme, const std::string& value, Position pos)
    : type_(type), lexeme_(lexeme), value_(value), position_(pos) {}

std::string Token::toString() const
{
    std::ostringstream oss;
    oss << "Token(" << token_type_to_string(type_) 
        << ", '" << lexeme_ << "'";
    if (lexeme_ != value_)
    {
        oss << ", value='" << value_ << "'";
    }
    oss << ", " << position_.line << ":" << position_.column << ")";
    return oss.str();
}

std::string token_type_to_string(TokenType type)
{
    switch (type)
    {
        case TokenType::EndOfFile: return "EOF";
        case TokenType::Integer: return "INTEGER";
        case TokenType::Float: return "FLOAT";
        case TokenType::Boolean: return "BOOLEAN";
        case TokenType::String: return "STRING";
        case TokenType::Identifier: return "IDENTIFIER";
        case TokenType::True: return "TRUE";
        case TokenType::False: return "FALSE";
        case TokenType::Dyna: return "DYNA";
        case TokenType::Color: return "COLOR";
        case TokenType::Coord: return "COORD";
        case TokenType::Path: return "PATH";
        case TokenType::List: return "LIST";
        case TokenType::Array: return "ARRAY";
        case TokenType::Plus: return "PLUS";
        case TokenType::Minus: return "MINUS";
        case TokenType::Star: return "STAR";
        case TokenType::Slash: return "SLASH";
        case TokenType::Percent: return "PERCENT";
        case TokenType::Assign: return "ASSIGN";
        case TokenType::PlusAssign: return "PLUS_ASSIGN";
        case TokenType::Colon: return "COLON";
        case TokenType::Comma: return "COMMA";
        case TokenType::Dot: return "DOT";
        case TokenType::Hash: return "HASH";
        case TokenType::At: return "AT";
        case TokenType::Dollar: return "DOLLAR";
        case TokenType::LeftBracket: return "LEFT_BRACKET";
        case TokenType::RightBracket: return "RIGHT_BRACKET";
        case TokenType::LeftParen: return "LEFT_PAREN";
        case TokenType::RightParen: return "RIGHT_PAREN";
        case TokenType::LeftBrace: return "LEFT_BRACE";
        case TokenType::RightBrace: return "RIGHT_BRACE";
        case TokenType::NewLine: return "NEWLINE";
        case TokenType::Comment: return "COMMENT";
        case TokenType::Section: return "SECTION";
        case TokenType::Define: return "DEFINE";
        case TokenType::Include: return "INCLUDE";
        case TokenType::Schema: return "SCHEMA";
        case TokenType::MacroRef: return "MACRO_REF";
        case TokenType::EnvVar: return "ENV_VAR";
        case TokenType::CrossRef: return "CROSS_REF";
        case TokenType::Unknown: return "UNKNOWN";
        default: return "UNKNOWN";
    }
}

} // namespace yini
