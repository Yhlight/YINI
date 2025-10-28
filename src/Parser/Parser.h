#ifndef YINI_PARSER_PARSER_H
#define YINI_PARSER_PARSER_H

#include "../Lexer/Token.h"
#include "ASTNode.h"
#include <vector>
#include <memory>

namespace yini
{

// Parser (placeholder)
class Parser
{
public:
    explicit Parser(const std::vector<Token>& tokens);
    std::shared_ptr<ASTNode> parse();
    
private:
    std::vector<Token> tokens_;
    size_t current_;
};

} // namespace yini

#endif // YINI_PARSER_PARSER_H
