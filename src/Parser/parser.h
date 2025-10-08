#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <memory>

struct Array;
struct Set;
struct Map;

using ConfigValue = std::variant<std::string, int, double, bool, std::unique_ptr<Array>, std::unique_ptr<Set>, std::unique_ptr<Map>>;

struct Array {
    std::vector<ConfigValue> elements;

    Array() = default;
    Array(const Array& other);

    bool operator==(const Array& other) const;
};

struct Set {
    std::vector<ConfigValue> elements;

    Set() = default;
    Set(const Set& other);

    bool operator==(const Set& other) const;
};

struct Map {
    std::map<std::string, ConfigValue> elements;

    Map() = default;
    Map(const Map& other);

    bool operator==(const Map& other) const;
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
    std::map<std::string, ConfigValue> macroMap;

    void nextToken();
    void expect(TokenType type);
    ConfigValue parseValue();
    std::unique_ptr<Array> parseArray();
    std::unique_ptr<Set> parseSet();
    std::unique_ptr<Map> parseMap();
    void resolveInheritance(Config& config);
};

#endif // PARSER_H