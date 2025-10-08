#include "Parser.h"
#include <stdexcept>
#include <string>
#include <iostream>
#include <vector>

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

    bool isSpecial = match({TokenType::Hash});
    Token name = consume(TokenType::Identifier, "Expect section name.");

    // This is the corrected logic: always consume the right bracket immediately.
    consume(TokenType::RightBracket, "Expect ']' after section name.");

    auto section = std::make_unique<SectionNode>(name);

    if (isSpecial) {
        if (name.lexeme == "define") {
            section->special_type = SpecialSectionType::Define;
        } else if (name.lexeme == "include") {
            section->special_type = SpecialSectionType::Include;
        }
    } else {
        // For regular sections, check for inheritance *after* the ']' has been consumed.
        if (match({TokenType::Colon}))
        {
            do {
                section->parents.push_back(consume(TokenType::Identifier, "Expect parent section name."));
            } while (match({TokenType::Comma}));
        }
    }

    size_t quickRegIndex = 0;
    // The loop condition is critical. It must stop if it sees the start of a new section.
    while (!isAtEnd() && (peek().type == TokenType::Identifier || peek().type == TokenType::PlusEqual))
    {
        section->pairs.push_back(parseKeyValuePair(quickRegIndex));
    }

    return section;
}

std::unique_ptr<Value> Parser::parseValue() {
    if (match({TokenType::At})) {
        Token refToken = consume(TokenType::Identifier, "Expect identifier after '@'.");
        return std::make_unique<ReferenceValue>(refToken);
    }
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
    if (match({TokenType::HexColor})) {
        std::string hex = previous().lexeme;
        if (hex.length() != 7 || hex[0] != '#') {
            throw std::runtime_error("Invalid hex color format. Expected #RRGGBB.");
        }
        try {
            uint8_t r = std::stoul(hex.substr(1, 2), nullptr, 16);
            uint8_t g = std::stoul(hex.substr(3, 2), nullptr, 16);
            uint8_t b = std::stoul(hex.substr(5, 2), nullptr, 16);
            return std::make_unique<ColorValue>(r, g, b);
        } catch(const std::invalid_argument& e) {
            throw std::runtime_error("Invalid hex character in color code.");
        }
    }
    if (match({TokenType::LeftBracket})) { // Array
        auto array = std::make_unique<ArrayValue>();
        if (!check(TokenType::RightBracket)) {
            do {
                array->elements.push_back(parseValue());
            } while (match({TokenType::Comma}));
        }
        consume(TokenType::RightBracket, "Expect ']' after array elements.");
        return array;
    }
    if (match({TokenType::LeftParen})) { // Set
        auto set = std::make_unique<SetValue>();
        if (!check(TokenType::RightParen)) {
            do {
                set->elements.push_back(parseValue());
            } while (match({TokenType::Comma}));
        }
        consume(TokenType::RightParen, "Expect ')' after set elements.");
        return set;
    }
    if (match({TokenType::LeftBrace})) { // Map
        auto map = std::make_unique<MapValue>();
        if (!check(TokenType::RightBrace)) {
            do {
                Token key = consume(TokenType::Identifier, "Expect key in map.");
                consume(TokenType::Colon, "Expect ':' after key in map.");
                map->elements[key.lexeme] = parseValue();
            } while (match({TokenType::Comma}));
        }
        consume(TokenType::RightBrace, "Expect '}' after map elements.");
        return map;
    }

    if (check(TokenType::Color) || check(TokenType::Coord) || check(TokenType::Path) || check(TokenType::List) || check(TokenType::Array)) {
        Token keyword = advance();
        consume(TokenType::LeftParen, "Expect '(' after custom type keyword.");
        std::vector<std::unique_ptr<Value>> args;
        if (!check(TokenType::RightParen)) {
            do {
                args.push_back(parseValue());
            } while (match({TokenType::Comma}));
        }
        consume(TokenType::RightParen, "Expect ')' after arguments.");

        if (keyword.type == TokenType::Color) {
            if (args.size() < 3 || args.size() > 4) throw std::runtime_error("Color() requires 3 or 4 arguments.");
            uint8_t r = dynamic_cast<NumberValue&>(*args[0]).value;
            uint8_t g = dynamic_cast<NumberValue&>(*args[1]).value;
            uint8_t b = dynamic_cast<NumberValue&>(*args[2]).value;
            uint8_t a = (args.size() == 4) ? dynamic_cast<NumberValue&>(*args[3]).value : 255;
            return std::make_unique<ColorValue>(r, g, b, a);
        } else if (keyword.type == TokenType::Coord) {
            if (args.size() < 2 || args.size() > 3) throw std::runtime_error("Coord() requires 2 or 3 arguments.");
            double x = dynamic_cast<NumberValue&>(*args[0]).value;
            double y = dynamic_cast<NumberValue&>(*args[1]).value;
            double z = (args.size() == 3) ? dynamic_cast<NumberValue&>(*args[2]).value : 0.0;
            return std::make_unique<CoordValue>(x, y, z, args.size() == 3);
        } else if (keyword.type == TokenType::Path) {
             if (args.size() > 1) throw std::runtime_error("Path() takes 0 or 1 argument.");
             std::string p = "";
             if (!args.empty()) {
                p = dynamic_cast<StringValue&>(*args[0]).value;
             }
             return std::make_unique<PathValue>(p);
        } else { // List or Array
            auto array = std::make_unique<ArrayValue>();
            array->elements = std::move(args);
            return array;
        }
    }

    if (match({TokenType::Identifier})) {
        return std::make_unique<IdentifierValue>(previous());
    }

    throw std::runtime_error("Expect a value.");
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