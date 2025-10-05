#pragma once

#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "Token.h"

namespace YINI {
class Lexer {
public:
    Lexer(std::string_view source, std::string_view filepath = "");
    std::vector<Token> scanTokens();

private:
    void scanToken();
    bool match(char expected);
    char peek();
    char peekNext();
    char advance();
    bool isAtEnd();
    void addToken(TokenType type);
    void addToken(TokenType type, const YiniValue& literal);

    void blockComment();
    void string();
    void number();
    void identifier();

    std::string_view m_source;
    std::string m_filepath;
    std::vector<Token> m_tokens;
    int m_start = 0;
    int m_current = 0;
    int m_line = 1;
    int m_column = 1;
    int m_start_column = 1;
};
}  // namespace YINI