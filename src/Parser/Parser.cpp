#include "Parser.h"
#include <stdexcept>

namespace YINI
{
    Parser::Parser(std::vector<Token> tokens) : m_tokens(std::move(tokens)) {}

    std::vector<std::unique_ptr<Stmt>> Parser::parse()
    {
        std::vector<std::unique_ptr<Stmt>> statements;
        while (!isAtEnd())
        {
            statements.push_back(declaration());
        }
        return statements;
    }

    std::unique_ptr<Stmt> Parser::declaration()
    {
        return section();
    }

    std::unique_ptr<Section> Parser::section()
    {
        consume(TokenType::LEFT_BRACKET, "Expect '[' before section name.");
        Token name = consume(TokenType::IDENTIFIER, "Expect section name.");
        consume(TokenType::RIGHT_BRACKET, "Expect ']' after section name.");

        std::vector<std::unique_ptr<KeyValue>> values;
        while (!check(TokenType::LEFT_BRACKET) && !isAtEnd())
        {
            values.push_back(keyValue());
        }

        return std::make_unique<Section>(name, std::move(values));
    }

    std::unique_ptr<KeyValue> Parser::keyValue()
    {
        Token key = consume(TokenType::IDENTIFIER, "Expect key.");
        consume(TokenType::EQUAL, "Expect '=' after key.");
        std::unique_ptr<Expr> value = expression();
        return std::make_unique<KeyValue>(key, std::move(value));
    }

    std::unique_ptr<Expr> Parser::expression()
    {
        return term();
    }

    std::unique_ptr<Expr> Parser::term()
    {
        std::unique_ptr<Expr> expr = factor();
        while (match({TokenType::MINUS, TokenType::PLUS}))
        {
            Token op = previous();
            std::unique_ptr<Expr> right = factor();
            expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
        }
        return expr;
    }

    std::unique_ptr<Expr> Parser::factor()
    {
        std::unique_ptr<Expr> expr = unary();
        while (match({TokenType::SLASH, TokenType::STAR, TokenType::PERCENT}))
        {
            Token op = previous();
            std::unique_ptr<Expr> right = unary();
            expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
        }
        return expr;
    }

    std::unique_ptr<Expr> Parser::unary()
    {
        if (match({TokenType::MINUS}))
        {
            Token op = previous();
            std::unique_ptr<Expr> right = unary();
            return std::make_unique<Unary>(op, std::move(right));
        }
        return primary();
    }

    std::unique_ptr<Expr> Parser::primary()
    {
        if (match({TokenType::TRUE, TokenType::FALSE, TokenType::NUMBER, TokenType::STRING}))
        {
            return std::make_unique<Literal>(previous().literal);
        }

        if (match({TokenType::LEFT_PAREN}))
        {
            std::unique_ptr<Expr> expr = expression();
            consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
            return std::make_unique<Grouping>(std::move(expr));
        }

        if (match({TokenType::LEFT_BRACKET}))
        {
            std::vector<std::unique_ptr<Expr>> elements;
            if (!check(TokenType::RIGHT_BRACKET))
            {
                do
                {
                    elements.push_back(expression());
                } while (match({TokenType::COMMA}));
            }
            consume(TokenType::RIGHT_BRACKET, "Expect ']' after array elements.");
            return std::make_unique<Array>(std::move(elements));
        }

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
        if (isAtEnd()) return false;
        return peek().type == type;
    }

    Token Parser::advance()
    {
        if (!isAtEnd()) m_current++;
        return previous();
    }

    bool Parser::isAtEnd()
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
}