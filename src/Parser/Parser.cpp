#include "Parser.h"

namespace yini
{

Parser::Parser(const std::vector<Token>& tokens)
    : tokens_(tokens), current_(0)
{
    (void)current_; // Suppress unused warning for now
}

std::shared_ptr<ASTNode> Parser::parse()
{
    // TODO: Implement parser
    return nullptr;
}

} // namespace yini
