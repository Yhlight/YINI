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
    std::unique_ptr<AST::KeyValueStmt> key_value_statement();
    std::unique_ptr<AST::Expr> expression();
    std::unique_ptr<AST::Expr> primary();
    std::unique_ptr<AST::Expr> array();
    std::unique_ptr<AST::Expr> set();
    std::unique_ptr<AST::Expr> map();
    std::unique_ptr<AST::Expr> color();
    std::unique_ptr<AST::Expr> coord();

    bool match(const std::vector<TokenType>& types);
    Token consume(TokenType type, const std::string& message);
    bool check(TokenType type);
    Token advance();
    bool is_at_end();
    Token peek();
    Token previous();

    std::vector<Token> m_tokens;
    int m_current = 0;
};
}