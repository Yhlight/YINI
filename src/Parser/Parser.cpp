#include "Parser.h"
#include <stdexcept>
#include <string>
#include <iostream>

namespace Yini
{
Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens) {}

std::vector<std::unique_ptr<SectionNode>> Parser::parse()
{
    std::vector<std::unique_ptr<SectionNode>> sections;
    while (!isAtEnd())
    {
        if (peek().type == TokenType::Eof) break;
        sections.push_back(parseSection());
    }
    return sections;
}

std::unique_ptr<SectionNode> Parser::parseSection()
{
    consume(TokenType::LeftBracket, "Expect '[' before section name.");
    Token name = consume(TokenType::Identifier, "Expect section name.");
    consume(TokenType::RightBracket, "Expect ']' after section name.");

    auto section = std::make_unique<SectionNode>(name);

    if (match({TokenType::Colon}))
    {
        do {
            section->parents.push_back(consume(TokenType::Identifier, "Expect parent section name."));
        } while (match({TokenType::Comma}));
    }

    size_t quickRegIndex = 0;
    while (!isAtEnd() && !check(TokenType::LeftBracket))
    {
        section->pairs.push_back(parseKeyValuePair(quickRegIndex));
    }

    return section;
}

std::unique_ptr<Value> Parser::parseValue() {
    if (match({TokenType::String})) {
        return std::make_unique<StringValue>(std::get<std::string>(previous().literal));
    }
    if (match({TokenType::Number})) {
        return std::make_unique<NumberValue>(std::get<double>(previous().literal));
    }
    if (match({TokenType::True})) {
        return std::make_unique<BoolValue>(true);
    }
    if (match({TokenType::False})) {
        return std::make_unique<BoolValue>(false);
    }
    if (match({TokenType::Identifier})) {
        return std::make_unique<IdentifierValue>(previous());
    }

    if (match({TokenType::LeftBracket})) {
        auto array = std::make_unique<ArrayValue>();
        if (!check(TokenType::RightBracket)) {
            do {
                array->elements.push_back(parseValue());
            } while (match({TokenType::Comma}));
        }
        consume(TokenType::RightBracket, "Expect ']' after array elements.");
        return array;
    }

    if (match({TokenType::List, TokenType::Array})) {
        consume(TokenType::LeftParen, "Expect '(' after List/Array keyword.");
        auto array = std::make_unique<ArrayValue>();
        if (!check(TokenType::RightParen)) {
            do {
                array->elements.push_back(parseValue());
            } while (match({TokenType::Comma}));
        }
        consume(TokenType::RightParen, "Expect ')' after list/array elements.");
        return array;
    }

    throw std::runtime_error("Expect a value (string, number, boolean, identifier, or array).");
}

std::unique_ptr<KeyValuePairNode> Parser::parseKeyValuePair(size_t& quickRegIndex)
{
    if (match({TokenType::PlusEqual}))
    {
        auto value = parseValue();

        Token keyToken;
        keyToken.type = TokenType::Identifier;
        keyToken.lexeme = std::to_string(quickRegIndex++);

        return std::make_unique<KeyValuePairNode>(keyToken, std::move(value));
    }
    else
    {
        Token key = consume(TokenType::Identifier, "Expect key.");
        consume(TokenType::Equal, "Expect '=' after key.");
        auto value = parseValue();
        return std::make_unique<KeyValuePairNode>(key, std::move(value));
    }
}


bool Parser::isAtEnd()
{
    return peek().type == TokenType::Eof;
}

Token Parser::peek()
{
    return tokens[current];
}

Token Parser::previous()
{
    return tokens[current - 1];
}

Token Parser::advance()
{
    if (!isAtEnd()) {
        current++;
    }
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

Token Parser::consume(TokenType type, const std::string& message)
{
    if (check(type)) return advance();
    Token foundToken = peek();
    std::string type_str = std::to_string(static_cast<int>(foundToken.type));
    throw std::runtime_error(message + " (found type " + type_str + " with lexeme '" + foundToken.lexeme + "')");
}

} // namespace Yini