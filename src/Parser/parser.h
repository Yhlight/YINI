#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include <string>
#include <vector>
#include <map>

#include <variant>

using ConfigValue = std::variant<std::string, int, double, bool>;
using ConfigSection = std::map<std::string, ConfigValue>;
using Config = std::map<std::string, ConfigSection>;

class Parser {
public:
    Parser(Lexer& lexer);
    Config parse();

private:
    Lexer& lexer;
    Token currentToken;

    void nextToken();
};

#endif // PARSER_H