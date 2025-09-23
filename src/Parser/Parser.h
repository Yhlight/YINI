#pragma once

#include "../Lexer/Token.h"
#include "Ast.h"
#include "Strategies.h"
#include <vector>
#include <stdexcept>
#include <memory>

class ParserError : public std::runtime_error
{
public:
    ParserError(const Token& token, const std::string& message);
};

class Parser
{
public:
    Parser(std::vector<Token> tokens);
    ~Parser();
    YiniFile parse();

public:
    // Token stream management
    const Token& peek();
    const Token& previous();
    const Token& advance();
    bool isAtEnd();
    bool check(TokenType type);
    bool match(const std::vector<TokenType>& types);
    const Token& consume(TokenType type, const std::string& errorMessage);

public:
    // The new entry point for parsing any value
    YiniValue parseValue();

private:
    // State-machine based parsing methods
    void parseTopLevel();
    void parseSection();
    void parseDefineSection();
    void parseIncludeSection();
    void parseGenericSection();

    void parseSectionBody(YiniSection& section);
    void parseKeyValuePair(YiniSection& section);
    void parseQuickRegistration(YiniSection& section);

    const std::vector<Token> tokenStream_;
    size_t current_ = 0;
    YiniFile yiniFile_;
    std::vector<std::unique_ptr<ValueParsingStrategy>> value_strategies_;
};
