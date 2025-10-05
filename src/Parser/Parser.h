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
        std::vector<std::unique_ptr<Stmt>> parse();

    private:
        // Expression parsing methods based on precedence
        std::unique_ptr<Expr> expression();
        std::unique_ptr<Expr> term();
        std::unique_ptr<Expr> factor();
        std::unique_ptr<Expr> unary();
        std::unique_ptr<Expr> call();
        std::unique_ptr<Expr> primary();

        // Primary expression helpers
        std::unique_ptr<Expr> parse_env_variable();
        std::unique_ptr<Expr> parse_cross_reference();
        std::unique_ptr<Expr> parse_variable();
        std::unique_ptr<Expr> parse_grouping_or_set();
        std::unique_ptr<Expr> parse_array();
        std::unique_ptr<Expr> parse_map();

        std::unique_ptr<Stmt> declaration();
        std::unique_ptr<Stmt> statement();
        std::unique_ptr<Stmt> registration();
        std::unique_ptr<Stmt> defineSection();
        std::unique_ptr<Stmt> includeSection();
        std::unique_ptr<Stmt> schemaSection();
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