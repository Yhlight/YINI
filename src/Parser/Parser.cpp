#include "Parser.h"
#include <iostream>
#include <utility>

namespace YINI
{

Parser::Parser(std::vector<Token> tokens) : m_tokens(std::move(tokens)) {}

Document Parser::parse()
{
    while (!isAtEnd())
    {
        skipNewlines();
        if (isAtEnd()) break;

        if (peek().type == TokenType::LEFT_BRACKET)
        {
            parseSection();
        }
        else if (m_current_section != nullptr)
        {
            parseStatement();
        }
        else
        {
            // Ignore anything not in a section, for now.
            advance();
        }
    }
    return std::move(m_doc);
}

void Parser::parseSection()
{
    consume(TokenType::LEFT_BRACKET, "Expect '[' to start a section.");

    if (match({TokenType::HASH}))
    {
        Token id = consume(TokenType::IDENTIFIER, "Expect 'define' or 'include' after '[#'.");
        consume(TokenType::RIGHT_BRACKET, "Expect ']' after special section name.");
        if (id.literal == "define")
        {
            parseDefineSection();
        }
        else if (id.literal == "include")
        {
            parseIncludeSection();
        }
        else
        {
            // Error: unknown special section
            throw std::runtime_error("Unknown special section: " + id.literal);
        }
    }
    else
    {
        parseGenericSection();
    }
}

void Parser::parseDefineSection()
{
    skipNewlines();
    m_current_section = nullptr; // Defines are not in a regular section

    while (!isAtEnd() && peek().type != TokenType::LEFT_BRACKET)
    {
        if (peek().type == TokenType::IDENTIFIER) {
            KeyValuePair kvp = parseKeyValuePair();
            m_doc.defines[kvp.key] = std::move(kvp.value);
        }
        skipNewlines();
    }
}

void Parser::parseIncludeSection()
{
    skipNewlines();
    m_current_section = nullptr; // Includes are not in a regular section

    while (!isAtEnd() && peek().type != TokenType::LEFT_BRACKET)
    {
        if (match({TokenType::PLUS_EQUAL}))
        {
            Token file = consume(TokenType::STRING, "Expect string literal for include path.");
            m_doc.includes.push_back(file.literal);
        }
        else
        {
            // Error
            throw std::runtime_error("Include section only supports '+=' statements.");
        }
        skipNewlines();
    }
}

void Parser::parseGenericSection()
{
    Token name = consume(TokenType::IDENTIFIER, "Expect section name.");
    consume(TokenType::RIGHT_BRACKET, "Expect ']' after section header.");

    m_doc.sections.emplace_back();
    m_current_section = &m_doc.sections.back();
    m_current_section->name = name.literal;

    if (match({TokenType::COLON}))
    {
        do
        {
            Token inherited = consume(TokenType::IDENTIFIER, "Expect inherited section name.");
            m_current_section->inherited_sections.push_back(inherited.literal);
        } while (match({TokenType::COMMA}));
    }

    skipNewlines();

    while (!isAtEnd() && peek().type != TokenType::LEFT_BRACKET)
    {
        parseStatement();
        skipNewlines();
    }
}

void Parser::parseStatement()
{
    if (peek().type == TokenType::IDENTIFIER && m_tokens[m_current + 1].type == TokenType::EQUAL)
    {
        m_current_section->pairs.push_back(parseKeyValuePair());
    }
    else if (peek().type == TokenType::PLUS_EQUAL)
    {
        advance(); // consume '+='
        m_current_section->anonymous_values.push_back(parseValue());
    }
    else
    {
        // Ignore malformed lines for now
        advance();
    }
}

KeyValuePair Parser::parseKeyValuePair()
{
    Token key = consume(TokenType::IDENTIFIER, "Expect key name.");
    consume(TokenType::EQUAL, "Expect '=' after key.");
    Value value = parseValue();
    return {key.literal, std::move(value)};
}

Value Parser::parseValue()
{
    if (match({TokenType::STRING})) return previous().literal;
    if (match({TokenType::INTEGER})) return std::stoll(previous().literal);
    if (match({TokenType::FLOAT})) return std::stod(previous().literal);
    if (match({TokenType::BOOLEAN})) return previous().literal == "true";
    if (peek().type == TokenType::LEFT_BRACKET) return parseArray();
    if (peek().type == TokenType::LEFT_BRACE) return parseCoordinate();
    if (peek().type == TokenType::HASH) return parseColor();
    if (match({TokenType::AT})) {
        Token var_name = consume(TokenType::IDENTIFIER, "Expect variable name after '@'.");
        auto it = m_doc.defines.find(var_name.literal);
        if (it != m_doc.defines.end()) {
            const Value& val = it->second;
            if (std::holds_alternative<std::string>(val)) return std::get<std::string>(val);
            if (std::holds_alternative<long long>(val)) return std::get<long long>(val);
            if (std::holds_alternative<double>(val)) return std::get<double>(val);
            if (std::holds_alternative<bool>(val)) return std::get<bool>(val);
            throw std::runtime_error("Variable references can only be simple types (string, int, float, bool).");
        } else {
            throw std::runtime_error("Undefined variable: " + var_name.literal);
        }
    }

    if (match({TokenType::IDENTIFIER})) {
        return previous().literal;
    }

    throw std::runtime_error("Unexpected token when parsing value: " + peek().literal);
}

Value Parser::parseArray()
{
    consume(TokenType::LEFT_BRACKET, "Expect '[' to start an array.");
    auto array = std::make_unique<Array>();
    if (!check(TokenType::RIGHT_BRACKET))
    {
        do
        {
            array->elements.push_back(parseValue());
        } while (match({TokenType::COMMA}));
    }
    consume(TokenType::RIGHT_BRACKET, "Expect ']' to end an array.");
    return array;
}

Value Parser::parseCoordinate()
{
    consume(TokenType::LEFT_BRACE, "Expect '{' to start a coordinate.");
    auto coord = std::make_unique<Coordinate>();
    Value x_val = parseValue();
    consume(TokenType::COMMA, "Expect ',' between coordinate values.");
    Value y_val = parseValue();
    consume(TokenType::RIGHT_BRACE, "Expect '}' to end a coordinate.");

    if (auto* x_ll = std::get_if<long long>(&x_val)) {
        coord->x = *x_ll;
    } else if (auto* x_d = std::get_if<double>(&x_val)) {
        coord->x = *x_d;
    }

    if (auto* y_ll = std::get_if<long long>(&y_val)) {
        coord->y = *y_ll;
    } else if (auto* y_d = std::get_if<double>(&y_val)) {
        coord->y = *y_d;
    }

    return coord;
}

Value Parser::parseColor()
{
    consume(TokenType::HASH, "Expect '#' to start a color hex.");
    Token hex = consume(TokenType::IDENTIFIER, "Expect hex code after '#'.");
    auto color = std::make_unique<Color>();
    // placeholder parsing logic
    if (hex.literal.length() == 6) {
        //...
    }
    return color;
}


// --- Helper methods ---

Token Parser::advance()
{
    if (!isAtEnd()) m_current++;
    return previous();
}

Token Parser::peek()
{
    return m_tokens[m_current];
}

Token Parser::previous()
{
    return m_tokens[m_current - 1];
}

bool Parser::isAtEnd()
{
    return peek().type == TokenType::END_OF_FILE;
}

bool Parser::check(TokenType type)
{
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::match(std::vector<TokenType> types)
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
    throw std::runtime_error(message + " Got " + peek().literal);
}

void Parser::skipNewlines()
{
    while(match({TokenType::NEWLINE}));
}

} // namespace YINI
