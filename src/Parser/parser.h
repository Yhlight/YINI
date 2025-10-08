#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <memory>

struct Array;

using ConfigValue = std::variant<std::string, int, double, bool, std::unique_ptr<Array>>;

struct Array {
    std::vector<ConfigValue> elements;

    Array() = default; // Default constructor
    Array(const Array& other); // Deep copy constructor

    bool operator==(const Array& other) const;
};

using ConfigSection = std::map<std::string, ConfigValue>;
using Config = std::map<std::string, ConfigSection>;

class Parser {
public:
    Parser(Lexer& lexer);
    Config parse();

private:
    Lexer& lexer;
    Token currentToken;
    std::map<std::string, std::vector<std::string>> inheritanceMap;

    void nextToken();
    void expect(TokenType type);
    ConfigValue parseValue();
    std::unique_ptr<Array> parseArray();
    void resolveInheritance(Config& config);
};

#endif // PARSER_H