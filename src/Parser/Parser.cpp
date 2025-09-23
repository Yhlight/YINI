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
    // Order is important here. More specific strategies should come before more general ones.
    value_strategies_.push_back(std::make_unique<MacroRefStrategy>());
    value_strategies_.push_back(std::make_unique<BoolStrategy>());
    value_strategies_.push_back(std::make_unique<PathStrategy>());
    value_strategies_.push_back(std::make_unique<CoordStrategy>());
    value_strategies_.push_back(std::make_unique<ColorStrategy>());
    value_strategies_.push_back(std::make_unique<ObjectStrategy>());
    value_strategies_.push_back(std::make_unique<ArrayStrategy>());
    value_strategies_.push_back(std::make_unique<StringStrategy>());
    value_strategies_.push_back(std::make_unique<NumericExpressionStrategy>());
}

Parser::~Parser() = default;

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
    for (auto& strategy : value_strategies_)
    {
        auto result = strategy->try_parse(*this);
        if (result.has_value())
        {
            return result.value();
        }
    }
    throw ParserError(peek(), "Could not parse value.");
}
