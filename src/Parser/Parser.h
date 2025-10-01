#pragma once

#include "Lexer/Token.h"
#include "Ast.h"
#include <vector>
#include <memory>

namespace YINI
{
    class Parser
    {
    public:
        Parser(std::vector<Token> tokens);
        std::vector<std::unique_ptr<AstNode>> parse();

    private:
        std::unique_ptr<Section> section();
        std::unique_ptr<KeyValue> keyValue();

        bool match(const std::vector<TokenType>& types);
        Token consume(TokenType type, const std::string& message);
        bool check(TokenType type);
        Token advance();
        bool isAtEnd();
        Token peek();
        Token previous();

        std::vector<Token> m_tokens;
        int m_current = 0;
    };
}