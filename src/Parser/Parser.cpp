#include "Parser.h"
#include <stdexcept>

namespace YINI
{
    Parser::Parser(std::vector<Token> tokens) : m_tokens(std::move(tokens)) {}

    std::vector<std::unique_ptr<AstNode>> Parser::parse()
    {
        std::vector<std::unique_ptr<AstNode>> statements;
        while (!isAtEnd())
        {
            statements.push_back(section());
        }
        return statements;
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
        Token value = consume(TokenType::STRING, "Expect value.");
        return std::make_unique<KeyValue>(key, value);
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