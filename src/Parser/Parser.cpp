#include "Parser.h"
#include <iostream>

namespace YINI
{
    Parser::Parser(const std::vector<Token>& tokens) : m_tokens(tokens)
    {
    }

    std::vector<std::unique_ptr<Stmt>> Parser::parse()
    {
        std::vector<std::unique_ptr<Stmt>> statements;
        while (!isAtEnd())
        {
            auto stmt = declaration();
            if (stmt) {
                statements.push_back(std::move(stmt));
            }
        }
        return statements;
    }

    std::unique_ptr<Stmt> Parser::declaration()
    {
        if (match({TokenType::LBracket})) return section();

        auto stmt = statement();
        if (stmt) return stmt;

        if (!isAtEnd()) {
            error(peek(), "Invalid statement.");
            advance();
        }
        return nullptr;
    }

    std::unique_ptr<Stmt> Parser::section()
    {
        Token name;
        if (match({TokenType::Hash}))
        {
            Token hash = previous();
            Token id = consume(TokenType::Identifier, "Expect 'define' or 'include' after '#'.");
            name = Token{TokenType::Identifier, hash.lexeme + id.lexeme, {}, id.line};
        }
        else
        {
            name = consume(TokenType::Identifier, "Expect section name.");
        }
        consume(TokenType::RBracket, "Expect ']' after section name.");

        std::vector<Token> inheritance;
        if (match({TokenType::Colon}))
        {
            do {
                if (check(TokenType::Identifier)) {
                    inheritance.push_back(advance());
                } else {
                    error(peek(), "Expect parent section name.");
                    break;
                }
            } while(match({TokenType::Comma}));
        }

        auto sectionNode = std::make_unique<SectionStmt>(name, inheritance);

        while (!check(TokenType::LBracket) && !isAtEnd()) {
            sectionNode->statements.push_back(statement());
        }

        return sectionNode;
    }

    std::unique_ptr<Stmt> Parser::statement()
    {
        if (peek().type == TokenType::Identifier && m_tokens[m_current + 1].type == TokenType::Equals)
        {
            return keyValueStatement();
        }
        if (peek().type == TokenType::PlusEquals)
        {
            return registerStatement();
        }
        // Other statement types will go here
        advance(); // Consume token to avoid infinite loop on error
        return nullptr;
    }

    std::unique_ptr<Stmt> Parser::registerStatement()
    {
        Token op = consume(TokenType::PlusEquals, "Expect '+='.");
        std::unique_ptr<Expr> value = expression();
        return std::make_unique<RegisterStmt>(op, std::move(value));
    }

    std::unique_ptr<Stmt> Parser::keyValueStatement()
    {
        Token key = consume(TokenType::Identifier, "Expect identifier (key).");
        consume(TokenType::Equals, "Expect '=' after key.");
        std::unique_ptr<Expr> value = expression();
        return std::make_unique<KeyValueStmt>(key, std::move(value));
    }

    std::unique_ptr<Expr> Parser::expression()
    {
        return term();
    }

    std::unique_ptr<Expr> Parser::term()
    {
        std::unique_ptr<Expr> expr = factor();

        while (match({TokenType::Minus, TokenType::Plus}))
        {
            Token op = previous();
            std::unique_ptr<Expr> right = factor();
            expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
        }

        return expr;
    }

    std::unique_ptr<Expr> Parser::factor()
    {
        std::unique_ptr<Expr> expr = call();

        while (match({TokenType::Slash, TokenType::Star, TokenType::Percent}))
        {
            Token op = previous();
            std::unique_ptr<Expr> right = call();
            expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
        }

        return expr;
    }

    std::unique_ptr<Expr> Parser::call()
    {
        std::unique_ptr<Expr> expr = primary();

        if (match({TokenType::LParen})) {
            LiteralExpr* callee = dynamic_cast<LiteralExpr*>(expr.get());
            if (callee && (callee->token.lexeme == "Dyna" || callee->token.lexeme == "dyna"))
            {
                std::unique_ptr<Expr> value = expression();
                consume(TokenType::RParen, "Expect ')' after Dyna value.");
                return std::make_unique<DynaExpr>(std::move(value));
            }
            else
            {
                expr = finishCall(std::move(expr));
                while (match({TokenType::LParen})) {
                    expr = finishCall(std::move(expr));
                }
            }
        }

        return expr;
    }

    std::unique_ptr<Expr> Parser::finishCall(std::unique_ptr<Expr> callee)
    {
        std::vector<std::unique_ptr<Expr>> arguments;
        if (!check(TokenType::RParen)) {
            do {
                arguments.push_back(expression());
            } while (match({TokenType::Comma}));
        }

        Token paren = consume(TokenType::RParen, "Expect ')' after arguments.");
        return std::make_unique<CallExpr>(std::move(callee), paren, std::move(arguments));
    }

    std::unique_ptr<Expr> Parser::primary()
    {
        if (match({TokenType::String, TokenType::Integer, TokenType::Float, TokenType::True, TokenType::False, TokenType::Coord, TokenType::Color, TokenType::Path, TokenType::Dyna, TokenType::Identifier}))
        {
            return std::make_unique<LiteralExpr>(previous());
        }

        if (match({TokenType::LParen}))
        {
            std::unique_ptr<Expr> expr = expression();
            consume(TokenType::RParen, "Expect ')' after expression.");
            return std::make_unique<GroupingExpr>(std::move(expr));
        }

        if (match({TokenType::LBracket}))
        {
            return array();
        }

        if (match({TokenType::LBrace}))
        {
            return map();
        }

        // If no expression matches, it's an error.
        error(peek(), "Expect expression.");
        return nullptr;
    }

    std::unique_ptr<Expr> Parser::map()
    {
        std::vector<std::unique_ptr<KeyValuePairExpr>> pairs;
        if (!check(TokenType::RBrace))
        {
            do {
                Token key = consume(TokenType::Identifier, "Expect key in map element.");
                consume(TokenType::Colon, "Expect ':' after key in map element.");
                std::unique_ptr<Expr> value = expression();
                pairs.push_back(std::make_unique<KeyValuePairExpr>(key, std::move(value)));
            } while (match({TokenType::Comma}));
        }
        consume(TokenType::RBrace, "Expect '}' to close map.");
        return std::make_unique<MapExpr>(std::move(pairs));
    }

    std::unique_ptr<Expr> Parser::array()
    {
        std::vector<std::unique_ptr<Expr>> elements;
        if (!check(TokenType::RBracket))
        {
            do {
                elements.push_back(expression());
            } while (match({TokenType::Comma}));
        }
        consume(TokenType::RBracket, "Expect ']' after array elements.");
        return std::make_unique<ArrayExpr>(std::move(elements));
    }

    Token Parser::consume(TokenType type, const std::string& message)
    {
        if (check(type)) return advance();
        error(peek(), message);
        // This is a dummy token. A real implementation would throw an exception
        // or enter a panic mode to synchronize.
        return Token{TokenType::Unknown, "", {}, peek().line};
    }

    void Parser::error(const Token& token, const std::string& message)
    {
        m_hasError = true;
        // For now, just print the error. A more robust implementation would
        // collect errors.
        std::cerr << "[line " << token.line << "] Error";
        if (token.type == TokenType::EndOfFile) {
            std::cerr << " at end";
        } else {
            std::cerr << " at '" << token.lexeme << "'";
        }
        std::cerr << ": " << message << std::endl;
    }

    bool Parser::hasError() const
    {
        return m_hasError;
    }

    bool Parser::isAtEnd()
    {
        return peek().type == TokenType::EndOfFile;
    }

    Token Parser::peek()
    {
        return m_tokens[m_current];
    }

    Token Parser::previous()
    {
        return m_tokens[m_current - 1];
    }

    Token Parser::advance()
    {
        if (!isAtEnd()) m_current++;
        return previous();
    }

    bool Parser::check(TokenType type)
    {
        if (isAtEnd()) return false;
        return peek().type == type;
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
}