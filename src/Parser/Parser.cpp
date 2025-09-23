#include "Parser.h"
#include <iostream>
#include <sstream>

// --- ParserError Implementation ---
ParserError::ParserError(const Token& token, const std::string& message)
    : std::runtime_error(
        "Parse Error at line " + std::to_string(token.line) +
        " column " + std::to_string(token.column) +
        ": " + message + " (found token '" + token.lexeme + "')"
    )
{
}

// --- Parser Implementation ---
Parser::Parser(std::vector<Token> tokens)
    : tokenStream_(std::move(tokens))
{
}

YiniFile Parser::parse()
{
    while (!isAtEnd())
    {
        parseTopLevel();
    }
    return yiniFile_;
}

// --- Token Stream Management ---
const Token& Parser::peek()
{
    return tokenStream_[current_];
}
const Token& Parser::previous()
{
    return tokenStream_[current_ - 1];
}
bool Parser::isAtEnd()
{
    return peek().type == TokenType::END_OF_FILE;
}
const Token& Parser::advance()
{
    if (!isAtEnd())
    {
        current_++;
    }
    return previous();
}

bool Parser::check(TokenType type)
{
    if (isAtEnd())
    {
        return false;
    }
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

const Token& Parser::consume(TokenType type, const std::string& errorMessage)
{
    if (check(type))
    {
        return advance();
    }
    throw ParserError(peek(), errorMessage);
}

// --- Parsing Rules ---
void Parser::parseTopLevel()
{
    if (match({TokenType::L_BRACKET}))
    {
        parseSection();
    }
    else
    {
        throw ParserError(peek(), "Expected '[' to start a section.");
    }
}

void Parser::parseSection()
{
    if (match({TokenType::HASH}))
    {
        const Token& type = consume(TokenType::IDENTIFIER, "Expected 'define' or 'include' after '#'.");
        if (type.lexeme == "define")
        {
            parseDefineSection();
        }
        else if (type.lexeme == "include")
        {
            parseIncludeSection();
        }
        else
        {
            throw ParserError(type, "Unknown special section type: " + type.lexeme);
        }
    }
    else
    {
        parseGenericSection();
    }
}

void Parser::parseDefineSection()
{
    consume(TokenType::R_BRACKET, "Expected ']' after '#define'.");
    while (!isAtEnd() && !check(TokenType::L_BRACKET))
    {
        const Token& key = consume(TokenType::IDENTIFIER, "Expected identifier for define key.");
        consume(TokenType::EQUAL, "Expected '=' after define key.");
        yiniFile_.definesMap[key.lexeme] = parseValue();
    }
}

void Parser::parseIncludeSection()
{
    consume(TokenType::R_BRACKET, "Expected ']' after '#include'.");
    while (!isAtEnd() && !check(TokenType::L_BRACKET))
    {
        consume(TokenType::PLUS_EQUAL, "Expected '+=' for include entry.");
        const Token& path = consume(TokenType::STRING, "Expected string for file path.");
        yiniFile_.includePaths.push_back(path.lexeme);
    }
}

void Parser::parseGenericSection()
{
    YiniSection section;
    section.name = consume(TokenType::IDENTIFIER, "Expected section name.").lexeme;
    consume(TokenType::R_BRACKET, "Expected ']' to close section name.");

    if (match({TokenType::COLON}))
    {
        do {
            section.inherits.push_back(consume(TokenType::IDENTIFIER, "Expected parent name for inheritance.").lexeme);
        } while (match({TokenType::COMMA}));
    }

    parseSectionBody(section);
    yiniFile_.sectionsMap[section.name] = section;
}

void Parser::parseSectionBody(YiniSection& section)
{
    while (!isAtEnd() && !check(TokenType::L_BRACKET))
    {
        if (check(TokenType::PLUS_EQUAL))
        {
            parseQuickRegistration(section);
        }
        else if (check(TokenType::IDENTIFIER))
        {
            parseKeyValuePair(section);
        }
        else
        {
            throw ParserError(peek(), "Expected key-value pair or quick registration.");
        }
    }
}

void Parser::parseKeyValuePair(YiniSection& section)
{
    const Token& key = consume(TokenType::IDENTIFIER, "Expected key.");
    consume(TokenType::EQUAL, "Expected '=' after key.");
    section.keyValues[key.lexeme] = parseValue();
}

void Parser::parseQuickRegistration(YiniSection& section)
{
    consume(TokenType::PLUS_EQUAL, "Expected '+='.");
    section.autoIndexedValues.push_back(parseValue());
}

YiniValue Parser::parseValue()
{
    if (match({TokenType::AT}))
    {
        const Token& name = consume(TokenType::IDENTIFIER, "Expected macro name after '@'.");
        return YiniValue{YiniMacroRef{name.lexeme}};
    }

    if (peek().type == TokenType::IDENTIFIER)
    {
        if (peek().lexeme == "true" || peek().lexeme == "false")
        {
            bool value = advance().lexeme == "true";
            return YiniValue{value};
        }
        if (peek().lexeme == "Coord" || peek().lexeme == "coord")
        {
            return parseCoord();
        }
        if (peek().lexeme == "Color" || peek().lexeme == "color")
        {
            return parseColor();
        }
    }

    if (match({TokenType::HASH}))
    {
        return parseColor();
    }

    if (check(TokenType::L_BRACE))
    {
        return parseObject();
    }

    if (match({TokenType::L_BRACKET}))
    {
        return parseArray();
    }

    return parsePrimary();
}

YiniValue Parser::parsePrimary()
{
    if (match({TokenType::STRING}))
    {
        return YiniValue{previous().lexeme};
    }
    if (match({TokenType::INTEGER}))
    {
        return YiniValue{std::stoll(previous().lexeme)};
    }
    if (match({TokenType::FLOAT}))
    {
        return YiniValue{std::stod(previous().lexeme)};
    }

    throw ParserError(peek(), "Expected a value.");
}

YiniValue Parser::parseArray()
{
    YiniArray array;
    if (!check(TokenType::R_BRACKET))
    {
        do
        {
            array.push_back(parseValue());
        } while (match({TokenType::COMMA}) && !check(TokenType::R_BRACKET));
    }
    consume(TokenType::R_BRACKET, "Expected ']' after array elements.");
    return YiniValue{array};
}

YiniValue Parser::parseCoord()
{
    consume(TokenType::IDENTIFIER, "Expected 'Coord' or 'coord'.");
    consume(TokenType::L_PAREN, "Expected '(' after 'Coord'.");
    YiniCoord coord;

    const Token& x = consume(TokenType::INTEGER, "Expected integer for x coordinate.");
    consume(TokenType::COMMA, "Expected comma after x coordinate.");
    const Token& y = consume(TokenType::INTEGER, "Expected integer for y coordinate.");

    coord.x = std::stod(x.lexeme);
    coord.y = std::stod(y.lexeme);

    if(match({TokenType::COMMA}))
    {
        const Token& z = consume(TokenType::INTEGER, "Expected integer for z coordinate.");
        coord.z = std::stod(z.lexeme);
        coord.is_3d = true;
    }

    consume(TokenType::R_PAREN, "Expected ')' after coordinates.");
    return YiniValue{coord};
}

YiniValue Parser::parseColor()
{
    YiniColor color;
    if (previous().type == TokenType::HASH)
    {
        const Token& hex = consume(TokenType::IDENTIFIER, "Expected hex code after '#'.");
        if (hex.lexeme.length() != 6)
        {
            throw ParserError(hex, "Hex color code must be 6 characters long.");
        }
        try
        {
            color.r = std::stoi(hex.lexeme.substr(0, 2), nullptr, 16);
            color.g = std::stoi(hex.lexeme.substr(2, 2), nullptr, 16);
            color.b = std::stoi(hex.lexeme.substr(4, 2), nullptr, 16);
        }
        catch(const std::invalid_argument& e)
        {
            throw ParserError(hex, "Invalid hex color code.");
        }
    }
    else
    {
        consume(TokenType::IDENTIFIER, "Expected 'color' identifier.");
        consume(TokenType::L_PAREN, "Expected '(' after 'color'.");
        const Token& r = consume(TokenType::INTEGER, "Expected integer for red value.");
        consume(TokenType::COMMA, "Expected comma after red value.");
        const Token& g = consume(TokenType::INTEGER, "Expected integer for green value.");
        consume(TokenType::COMMA, "Expected comma after green value.");
        const Token& b = consume(TokenType::INTEGER, "Expected integer for blue value.");
        consume(TokenType::R_PAREN, "Expected ')' after color values.");
        color.r = std::stoi(r.lexeme);
        color.g = std::stoi(g.lexeme);
        color.b = std::stoi(b.lexeme);
    }
    return YiniValue{color};
}

YiniValue Parser::parseObject()
{
    consume(TokenType::L_BRACE, "Expected '{' to start object.");
    YiniObject obj;
    if (!check(TokenType::R_BRACE))
    {
        do
        {
            const Token& key = consume(TokenType::IDENTIFIER, "Expected key in object.");
            consume(TokenType::COLON, "Expected ':' after key in object.");
            obj[key.lexeme] = parseValue();
        } while (match({TokenType::COMMA}) && !check(TokenType::R_BRACE));
    }
    consume(TokenType::R_BRACE, "Expected '}' after object.");
    return YiniValue{obj};
}
