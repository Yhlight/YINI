#pragma once

#include "Ast.h"
#include "../Lexer/Token.h"
#include <vector>
#include <memory>

namespace YINI
{
    class Parser
    {
    public:
        Parser(const std::vector<Token>& tokens);
        std::vector<std::unique_ptr<Stmt>> parse();
        bool hasError() const;

    private:
        const std::vector<Token>& m_tokens;
        int m_current = 0;
        bool m_hasError = false;

        bool isAtEnd();
        Token peek();
        Token previous();
        Token advance();
        bool check(TokenType type);
        bool match(const std::vector<TokenType>& types);
        Token consume(TokenType type, const std::string& message);

        // Parsing methods
        std::unique_ptr<Stmt> declaration();
        std::unique_ptr<Stmt> section();
        std::unique_ptr<Stmt> statement();
        std::unique_ptr<Stmt> keyValueStatement();
        std::unique_ptr<Stmt> registerStatement();
        std::unique_ptr<Expr> expression();
        std::unique_ptr<Expr> term();
        std::unique_ptr<Expr> factor();
        std::unique_ptr<Expr> call();
        std::unique_ptr<Expr> primary();
        std::unique_ptr<Expr> array();
        std::unique_ptr<Expr> map();
        std::unique_ptr<Expr> finishCall(std::unique_ptr<Expr> callee);

        void error(const Token& token, const std::string& message);
    };
}