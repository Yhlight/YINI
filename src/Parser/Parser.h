#pragma once

#include "../Lexer/Token.h"
#include "Ast.h"
#include <vector>
#include <stdexcept>

class ParserError : public std::runtime_error
{
public:
    ParserError(const Token& token, const std::string& message);
};

class Parser
{
public:
    Parser(std::vector<Token> tokens);
    YiniFile parse();

private:
    // Token stream management
    const Token& peek();
    const Token& previous();
    const Token& advance();
    bool isAtEnd();
    bool check(TokenType type);
    bool match(const std::vector<TokenType>& types);
    const Token& consume(TokenType type, const std::string& errorMessage);

    // Parsing methods
    void parseTopLevel();
    void parseSection();
    void parseDefineSection();
    void parseIncludeSection();
    void parseGenericSection();

    void parseSectionBody(YiniSection& section);
    void parseKeyValuePair(YiniSection& section);
    void parseQuickRegistration(YiniSection& section);

    YiniValue parseValue();
    YiniValue parsePrimary();
    YiniValue parseArray();
    YiniValue parseCoord();
    YiniValue parseColor();
    YiniValue parseObject();

    const std::vector<Token> tokenStream_;
    size_t current_ = 0;
    YiniFile yiniFile_;
};
