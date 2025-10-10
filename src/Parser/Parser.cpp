#include "Parser.h"
#include <stdexcept>

namespace YINI
{

Parser::Parser(const std::vector<Token>& tokens) : m_tokens(tokens)
{
}

std::vector<std::unique_ptr<AST::Stmt>> Parser::parse()
{
    std::vector<std::unique_ptr<AST::Stmt>> statements;
    while (!is_at_end())
    {
        statements.push_back(declaration());
    }
    return statements;
}

std::unique_ptr<AST::Stmt> Parser::declaration()
{
    if (match({TokenType::LEFT_BRACKET}))
    {
        return section_declaration();
    }
    // In a valid YINI file, key-value pairs should be inside sections.
    // However, for now, we'll allow them at the top level for simpler parsing.
    // This will be refined later.
    return key_value_statement();
}

std::unique_ptr<AST::Stmt> Parser::section_declaration()
{
    Token name = consume(TokenType::IDENTIFIER, "Expect section name.");
    consume(TokenType::RIGHT_BRACKET, "Expect ']' after section name.");

    auto section = std::make_unique<AST::SectionStmt>();
    section->name = name;

    while (!check(TokenType::LEFT_BRACKET) && !is_at_end())
    {
        section->statements.push_back(key_value_statement());
    }

    return section;
}

std::unique_ptr<AST::Stmt> Parser::key_value_statement()
{
    Token key = consume(TokenType::IDENTIFIER, "Expect key.");
    consume(TokenType::EQUAL, "Expect '=' after key.");
    auto value = expression();

    auto stmt = std::make_unique<AST::KeyValueStmt>();
    stmt->key = key;
    stmt->value = std::move(value);

    return stmt;
}

std::unique_ptr<AST::Expr> Parser::expression()
{
    return primary();
}

std::unique_ptr<AST::Expr> Parser::primary()
{
    if (match({TokenType::STRING, TokenType::NUMBER}))
    {
        auto literal = std::make_unique<AST::LiteralExpr>();
        literal->value = previous();
        return literal;
    }

    // For now, we'll just throw an error for unsupported expressions.
    throw std::runtime_error("Expect expression.");
}

bool Parser::match(const std::vector<TokenType>& types)
{
    for (TokenType type : types)
    {
        if (check(type))
        {
            advance();
            return true;
        }
    }
    return false;
}

Token Parser::consume(TokenType type, const std::string& message)
{
    if (check(type)) return advance();
    throw std::runtime_error(message);
}

bool Parser::check(TokenType type)
{
    if (is_at_end()) return false;
    return peek().type == type;
}

Token Parser::advance()
{
    if (!is_at_end()) m_current++;
    return previous();
}

bool Parser::is_at_end()
{
    return peek().type == TokenType::END_OF_FILE;
}

Token Parser::peek()
{
    return m_tokens[m_current];
}

Token Parser::previous()
{
    return m_tokens[m_current - 1];
}

} // namespace YINI