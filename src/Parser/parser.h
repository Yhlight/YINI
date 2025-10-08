#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <memory>
#include <optional>

// Forward Declarations
struct Array;
struct Set;
struct Map;

// Complete Type Definitions needed for ConfigValue and SchemaRule
struct CrossSectionRef {
    std::string section;
    std::string key;
};

struct Color {
    int r, g, b;
    bool operator==(const Color& other) const { return r == other.r && g == other.g && b == other.b; }
};

struct Coord {
    int x, y, z;
    bool operator==(const Coord& other) const { return x == other.x && y == other.y && z == other.z; }
};

struct Path {
    std::string value;
    bool operator==(const Path& other) const { return value == other.value; }
};

using ConfigValue = std::variant<std::string, int, double, bool, std::unique_ptr<Array>, std::unique_ptr<Set>, std::unique_ptr<Map>, CrossSectionRef, Color, Coord, Path>;

struct SchemaRule {
    bool required = false;
    std::optional<std::string> type;
    std::optional<ConfigValue> default_value;
    char empty_behavior = '~'; // ~, =, e
    std::optional<double> min_val;
    std::optional<double> max_val;
};

using Schema = std::map<std::string, std::map<std::string, SchemaRule>>;

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
    Parser() = default;
    Parser(Lexer& lexer);
    Config parse(const std::string& input);
    Config parseFile(const std::string& filepath);
    const Schema& getSchema() const;
    void validate(Config& config) const;

private:
    Lexer* lexer;
    Token currentToken;
    Config config;
    Schema schema;
    std::map<std::string, std::vector<std::string>> inheritanceMap;
    std::map<std::string, ConfigValue> macroMap;
    bool in_schema_mode = false;
    std::string current_schema_target_section;

    void _parse(Lexer& lexer_ref, const std::string& current_dir);
    void nextToken();
    void expect(TokenType type);
    SchemaRule parseSchemaRule();
    std::unique_ptr<Array> parseArray();
    std::unique_ptr<Set> parseSet();
    std::unique_ptr<Map> parseMap();
    void resolveInheritance(Config& config);
    void resolveReferences(Config& config);

    // Expression parsing
    ConfigValue parse_expression();
    ConfigValue parse_term();
    ConfigValue parse_factor();
    ConfigValue parse_primary();
};

#endif // PARSER_H