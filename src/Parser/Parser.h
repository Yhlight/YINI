#pragma once

#include "Lexer/Token.h"
#include "AST.h"
#include <vector>
#include <memory>

namespace YINI
{
class Parser
{
public:
    Parser(const std::vector<Token>& tokens);
    std::vector<std::unique_ptr<AST::Stmt>> parse();

private:
    std::unique_ptr<AST::Stmt> declaration();
    std::unique_ptr<AST::Stmt> section_declaration();
    std::unique_ptr<AST::Stmt> define_section_declaration();
    std::unique_ptr<AST::Stmt> include_section_declaration();
    std::unique_ptr<AST::Stmt> schema_declaration();
    std::unique_ptr<AST::SchemaSectionStmt> schema_section_declaration();
    std::unique_ptr<AST::SchemaRuleStmt> schema_rule_statement();
    std::unique_ptr<AST::KeyValueStmt> key_value_statement();
    std::unique_ptr<AST::QuickRegStmt> quick_reg_statement();
    std::unique_ptr<AST::Expr> expression();
    std::unique_ptr<AST::Expr> term();
    std::unique_ptr<AST::Expr> factor();
    std::unique_ptr<AST::Expr> primary();
    std::unique_ptr<AST::Expr> array();
    std::unique_ptr<AST::Expr> set();
    std::unique_ptr<AST::Expr> map();
    std::unique_ptr<AST::Expr> color();
    std::unique_ptr<AST::Expr> coord();
    std::unique_ptr<AST::Expr> dyna();
    std::unique_ptr<AST::Expr> path();
    std::unique_ptr<AST::Expr> list();

    bool match(const std::vector<TokenType>& types);
    Token consume(TokenType type, const std::string& message);
    bool check(TokenType type);
    Token advance();
    bool is_at_end();
    Token peek();
    Token peek_next();
    Token previous();

    std::vector<Token> m_tokens;
    int m_current = 0;
};
}