#ifndef YINI_PARSER_H
#define YINI_PARSER_H

#include "../Lexer/Token.h"
#include "AST.h"
#include <vector>
#include <map>

namespace YINI
{

class Parser
{
public:
    Parser(std::vector<Token> tokens);
    Document parse();

private:
    std::vector<Token> m_tokens;
    size_t m_current = 0;

    Document m_doc;
    Section* m_current_section = nullptr;

    void parseSection();
    void parseDefineSection();
    void parseIncludeSection();
    void parseGenericSection();
    void parseStatement();
    KeyValuePair parseKeyValuePair();
    Value parseValue();
    Value parseArray();
    Value parseCoordinate();
    Value parseColor();


    Token advance();
    Token peek();
    Token previous();
    bool isAtEnd();
    bool check(TokenType type);
    bool match(std::vector<TokenType> types);
    Token consume(TokenType type, const std::string& message);
    void skipNewlines();
};

} // namespace YINI

#endif // YINI_PARSER_H
