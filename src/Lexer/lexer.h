#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>

enum class TokenType {
    Identifier,
    Equals,
    PlusEquals,
    String,
    Number,
    Boolean,
    Comment,
    LeftBracket,
    RightBracket,
    LeftParen,
    RightParen,
    Plus,
    Minus,
    Star,
    Slash,
    Percent,
    LeftBrace,
    RightBrace,
    Comma,
    Colon,
    At,
    Dollar,
    Hash,
    Dot,
    Bang,
    Question,
    EndOfFile,
    Unexpected
};

struct Token {
    TokenType type;
    std::string value;
    size_t line;
    size_t column;
};

class Lexer {
public:
    Lexer(const std::string& input);
    Token nextToken();
    std::vector<Token> allTokens();

private:
    std::string input;
    size_t position;
    size_t line;
    size_t column_start;

    void skipWhitespace();
};

#endif // LEXER_H