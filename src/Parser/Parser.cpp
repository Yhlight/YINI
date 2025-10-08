#include "Parser.h"
#include <stdexcept>
#include <string>

namespace Yini
{
Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens) {}

std::vector<std::unique_ptr<SectionNode>> Parser::parse()
{
    std::vector<std::unique_ptr<SectionNode>> sections;
    while (!isAtEnd())
    {
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

std::unique_ptr<KeyValuePairNode> Parser::parseKeyValuePair(size_t& quickRegIndex)
{
    if (match({TokenType::PlusEqual}))
    {
        Token valueToken = consume(TokenType::Identifier, "Expect value after '+='.");
        Token keyToken;
        keyToken.type = TokenType::Identifier;
        keyToken.lexeme = std::to_string(quickRegIndex++);
        keyToken.line = valueToken.line;
        auto value = std::make_unique<ValueNode>(valueToken);
        return std::make_unique<KeyValuePairNode>(keyToken, std::move(value));
    }
    else
    {
        Token key = consume(TokenType::Identifier, "Expect key.");
        consume(TokenType::Equal, "Expect '=' after key.");
        Token valueToken = consume(TokenType::Identifier, "Expect value.");
        auto value = std::make_unique<ValueNode>(valueToken);
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
    if (!isAtEnd()) current++;
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