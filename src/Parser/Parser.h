#pragma once

#include "Lexer/Token.h"
#include "Parser/Ast.h"
#include <vector>
#include <memory>

namespace Yini
{
class Parser
{
public:
    Parser(const std::vector<Token>& tokens);
    std::vector<std::unique_ptr<SectionNode>> parse();

private:
    std::unique_ptr<SectionNode> parseSection();
    std::unique_ptr<KeyValuePairNode> parseKeyValuePair(size_t& quickRegIndex);

    bool isAtEnd();
    Token peek();
    Token previous();
    Token advance();
    bool check(TokenType type);
    bool match(const std::vector<TokenType>& types);
    Token consume(TokenType type, const std::string& message);

    const std::vector<Token>& tokens;
    int current = 0;
};
} // namespace Yini