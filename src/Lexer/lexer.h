#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>

enum class TokenType {
    Section,
    Identifier,
    Equals,
    String,
    Number,
    Boolean,
    Comment,
    EndOfFile,
    Unexpected
};

struct Token {
    TokenType type;
    std::string value;
};

class Lexer {
public:
    Lexer(const std::string& input);
    Token nextToken();

private:
    std::string input;
    size_t position;

    void skipWhitespace();
};

#endif // LEXER_H