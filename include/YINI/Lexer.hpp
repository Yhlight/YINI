#ifndef YINI_LEXER_HPP
#define YINI_LEXER_HPP

#include <string>
#include <vector>

namespace YINI
{
enum class TokenType
{
    Eof,
    Section,
    Key,
    Value,
    Equals,
    Comment,
    String,
    Number,
    Boolean,
    LeftBracket,
    RightBracket,
    LeftParen,
    RightParen,
    LeftBrace,
    RightBrace,
    Comma,
    Plus,
    Minus,
    Star,
    Slash,
    Percent,
    Colon,
    PlusEquals,
    At,
    Hash,
    HexColor,
    Identifier,
    Unknown
};

struct Token
{
    TokenType type;
    std::string value;
    int line;
    int column;
};

class Lexer
{
  public:
    Lexer(const std::string &input);
    Token getNextToken();

  private:
    void skipWhitespace();
    void skipComment();
    Token string();
    Token number();
    Token identifier();

    std::string m_input;
    size_t m_position;
    int m_line;
    int m_column;
};
} // namespace YINI

#endif // YINI_LEXER_HPP