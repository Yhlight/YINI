#pragma once

#include "Lexer/Token.h"
#include "AST.h"
#include <vector>
#include <memory>

namespace YINI
{

/**
 * @brief The Parser for the YINI language.
 * @details It takes a vector of tokens from the Lexer and produces an Abstract Syntax Tree (AST).
 *          This is a recursive descent parser.
 */
class Parser
{
public:
    /**
     * @brief Constructs a new Parser object.
     * @param tokens A vector of tokens produced by the Lexer.
     */
    Parser(const std::vector<Token>& tokens);

    /**
     * @brief Parses the sequence of tokens into a vector of AST statements.
     * @return A vector of unique_ptr to the root statements of the AST.
     * @throws std::runtime_error on a parsing error.
     */
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
    std::unique_ptr<AST::Expr> unary();
    std::unique_ptr<AST::Expr> primary();
    std::unique_ptr<AST::Expr> array();
    std::unique_ptr<AST::Expr> set();
    std::unique_ptr<AST::Expr> map();
    std::unique_ptr<AST::Expr> color();
    std::unique_ptr<AST::Expr> coord();
    std::unique_ptr<AST::Expr> dyna();
    std::unique_ptr<AST::Expr> path();
    std::unique_ptr<AST::Expr> list();
    std::unique_ptr<AST::Expr> array_func();

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
