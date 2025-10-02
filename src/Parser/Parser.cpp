#include "Parser.h"
#include "Core/YiniException.h"
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
        if (check(TokenType::LEFT_BRACKET) && m_tokens[m_current + 1].type == TokenType::IDENTIFIER) {
            if (m_tokens[m_current + 1].lexeme == "#define") {
                return defineSection();
            }
            if (m_tokens[m_current + 1].lexeme == "#include") {
                return includeSection();
            }
        }
        return section();
    }

    std::unique_ptr<Stmt> Parser::includeSection()
    {
        consume(TokenType::LEFT_BRACKET, "Expect '[' before #include.");
        consume(TokenType::IDENTIFIER, "Expect #include keyword.");
        consume(TokenType::RIGHT_BRACKET, "Expect ']' after #include.");

        std::vector<std::unique_ptr<Expr>> files;
        while (!check(TokenType::LEFT_BRACKET) && !isAtEnd())
        {
            auto reg_stmt = registration();
            auto* reg = dynamic_cast<Register*>(reg_stmt.get());
            if (reg) {
                 files.push_back(std::move(reg->value));
            } else {
                const Token& token = peek();
                throw YiniException("Expected '+=' statement inside [#include] block.", token.line, token.column, token.filepath);
            }
        }

        return std::make_unique<Include>(std::move(files));
    }

    std::unique_ptr<Stmt> Parser::defineSection()
    {
        consume(TokenType::LEFT_BRACKET, "Expect '[' before #define.");
        consume(TokenType::IDENTIFIER, "Expect #define keyword.");
        consume(TokenType::RIGHT_BRACKET, "Expect ']' after #define.");

        std::vector<std::unique_ptr<KeyValue>> values;
        while (!check(TokenType::LEFT_BRACKET) && !isAtEnd())
        {
            values.push_back(keyValue());
        }

        return std::make_unique<Define>(std::move(values));
    }

    std::unique_ptr<Section> Parser::section()
    {
        consume(TokenType::LEFT_BRACKET, "Expect '[' before section name.");
        Token name = consume(TokenType::IDENTIFIER, "Expect section name.");
        consume(TokenType::RIGHT_BRACKET, "Expect ']' after section name.");

        std::vector<Token> parents;
        if (match({TokenType::COLON}))
        {
            do
            {
                parents.push_back(consume(TokenType::IDENTIFIER, "Expect parent section name."));
            } while (match({TokenType::COMMA}));
        }

        std::vector<std::unique_ptr<Stmt>> statements;
        while (!check(TokenType::LEFT_BRACKET) && !isAtEnd())
        {
            statements.push_back(statement());
        }

        return std::make_unique<Section>(name, std::move(parents), std::move(statements));
    }

    std::unique_ptr<Stmt> Parser::statement()
    {
        if (peek().type == TokenType::PLUS_EQUAL)
        {
            return registration();
        }
        return keyValue();
    }

    std::unique_ptr<Stmt> Parser::registration()
    {
        consume(TokenType::PLUS_EQUAL, "Expect '+=' for registration.");
        std::unique_ptr<Expr> value = expression();
        return std::make_unique<Register>(std::move(value));
    }

    std::unique_ptr<KeyValue> Parser::keyValue()
    {
        Token key = consume(TokenType::IDENTIFIER, "Expect key.");
        consume(TokenType::EQUAL, "Expect '=' after key.");
        Token valueStartToken = peek();
        std::unique_ptr<Expr> value = expression();
        auto kv = std::make_unique<KeyValue>(key, std::move(value));
        kv->value_line = valueStartToken.line;
        kv->value_column = valueStartToken.column;
        return kv;
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
        return call();
    }

    std::unique_ptr<Expr> Parser::call()
    {
        std::unique_ptr<Expr> expr = primary();

        while (match({TokenType::LEFT_PAREN}))
        {
            std::vector<std::unique_ptr<Expr>> arguments;
            if (!check(TokenType::RIGHT_PAREN))
            {
                do
                {
                    arguments.push_back(expression());
                } while (match({TokenType::COMMA}));
            }
            Token paren = consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments.");
            expr = std::make_unique<Call>(std::move(expr), paren, std::move(arguments));
        }

        return expr;
    }

    std::unique_ptr<Expr> Parser::primary()
    {
        if (match({TokenType::TRUE, TokenType::FALSE, TokenType::NUMBER, TokenType::STRING}))
        {
            return std::make_unique<Literal>(previous().literal);
        }

        if (match({TokenType::AT}))
        {
            Token name = consume(TokenType::IDENTIFIER, "Expect variable name after '@'.");
            return std::make_unique<Variable>(name);
        }

        if (match({TokenType::IDENTIFIER}))
        {
            return std::make_unique<Literal>(previous().literal);
        }

        if (match({TokenType::LEFT_PAREN}))
        {
            if (match({TokenType::RIGHT_PAREN})) { // Empty set: ()
                return std::make_unique<Set>(std::vector<std::unique_ptr<Expr>>{});
            }

            auto firstExpr = expression();

            if (match({TokenType::COMMA})) { // It's a set
                std::vector<std::unique_ptr<Expr>> elements;
                elements.push_back(std::move(firstExpr));

                if (!check(TokenType::RIGHT_PAREN)) { // Handles trailing comma for single-element set
                    do {
                        elements.push_back(expression());
                    } while (match({TokenType::COMMA}));
                }
                consume(TokenType::RIGHT_PAREN, "Expect ')' after set elements.");
                return std::make_unique<Set>(std::move(elements));
            }

            consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
            return std::make_unique<Grouping>(std::move(firstExpr));
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

        if (match({TokenType::LEFT_BRACE}))
        {
            Token brace = previous();
            std::vector<std::pair<std::unique_ptr<Expr>, std::unique_ptr<Expr>>> pairs;
            if (!check(TokenType::RIGHT_BRACE))
            {
                do
                {
                    std::unique_ptr<Expr> key = expression();
                    consume(TokenType::COLON, "Expect ':' after map key.");
                    std::unique_ptr<Expr> value = expression();
                    pairs.push_back(std::make_pair(std::move(key), std::move(value)));
                } while (match({TokenType::COMMA}));
            }
            consume(TokenType::RIGHT_BRACE, "Expect '}' after map pairs.");
            return std::make_unique<Map>(brace, std::move(pairs));
        }

        const Token& token = peek();
        throw YiniException("Expect expression.", token.line, token.column, token.filepath);
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
        const Token& token = peek();
        throw YiniException(message, token.line, token.column, token.filepath);
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