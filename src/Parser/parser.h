#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <memory>
#include <optional>
#include <nlohmann/json.hpp>

// --- Forward Declarations ---
struct Array;
struct Set;
struct Map;
struct DynaValue;
struct CrossSectionRef;
struct Color;
struct Coord;
struct Path;

// --- Core Type Alias ---
// Defines the main variant type. It can use unique_ptr with incomplete types.
using ConfigValue = std::variant<
    std::string,
    int,
    double,
    bool,
    std::unique_ptr<Array>,
    std::unique_ptr<Set>,
    std::unique_ptr<Map>,
    CrossSectionRef,
    Color,
    Coord,
    Path,
    std::unique_ptr<DynaValue>
>;

// --- Structure Definitions ---

// By-value types for the variant need to be fully defined.
struct CrossSectionRef {
    std::string section;
    std::string key;
    bool operator==(const CrossSectionRef& other) const { return section == other.section && key == other.key; }
    bool operator!=(const CrossSectionRef& other) const { return !(*this == other); }
};

struct Color {
    int r, g, b;
    bool operator==(const Color& other) const { return r == other.r && g == other.g && b == other.b; }
    bool operator!=(const Color& other) const { return !(*this == other); }
};

struct Coord {
    int x, y, z;
    bool operator==(const Coord& other) const { return x == other.x && y == other.y && z == other.z; }
    bool operator!=(const Coord& other) const { return !(*this == other); }
};

struct Path {
    std::string value;
    bool operator==(const Path& other) const { return value == other.value; }
    bool operator!=(const Path& other) const { return !(*this == other); }
};

// Recursive types must have their special members defined in the .cpp file.
struct Array {
    std::vector<ConfigValue> elements;
    Array() = default;
    Array(const Array& other);
    ~Array();
    Array(Array&&) noexcept;
    Array& operator=(const Array&);
    Array& operator=(Array&&) noexcept;
    bool operator==(const Array& other) const;
};

struct Set {
    std::vector<ConfigValue> elements;
    Set() = default;
    Set(const Set& other);
     ~Set();
    Set(Set&&) noexcept;
    Set& operator=(const Set&);
    Set& operator=(Set&&) noexcept;
    bool operator==(const Set& other) const;
};

struct Map {
    std::map<std::string, ConfigValue> elements;
    Map() = default;
    Map(const Map& other);
    ~Map();
    Map(Map&&) noexcept;
    Map& operator=(const Map&);
    Map& operator=(Map&&) noexcept;
    bool operator==(const Map& other) const;
};

struct DynaValue {
    std::unique_ptr<ConfigValue> value;
    std::vector<std::unique_ptr<ConfigValue>> backup;
    DynaValue();
    DynaValue(const DynaValue& other);
    DynaValue(DynaValue&& other) noexcept;
    DynaValue& operator=(const DynaValue& other);
    DynaValue& operator=(DynaValue&& other) noexcept;
    ~DynaValue();
    bool operator==(const DynaValue& other) const;
};

struct SchemaRule {
    bool required = false;
    std::optional<std::string> type;
    std::optional<ConfigValue> default_value;
    char empty_behavior = '~';
    std::optional<double> min_val;
    std::optional<double> max_val;
};

using Schema = std::map<std::string, std::map<std::string, SchemaRule>>;
using ConfigSection = std::map<std::string, ConfigValue>;
using Config = std::map<std::string, ConfigSection>;

// --- Parser Class ---
class Parser {
public:
    Parser() = default;
    Parser(Lexer& lexer);
    Config parse(const std::string& input);
    Config parseFile(const std::string& filepath);
    ConfigValue parseValue(const std::string& input);
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

    ConfigValue parse_expression();
    ConfigValue parse_term();
    ConfigValue parse_factor();
    ConfigValue parse_primary();
};

// --- JSON Serialization ---
void to_json(nlohmann::json& j, const ConfigValue& val);
void to_json(nlohmann::json& j, const Array& a);
void to_json(nlohmann::json& j, const Set& s);
void to_json(nlohmann::json& j, const Map& m);
void to_json(nlohmann::json& j, const CrossSectionRef& r);
void to_json(nlohmann::json& j, const Color& c);
void to_json(nlohmann::json& j, const Coord& c);
void to_json(nlohmann::json& j, const Path& p);
void to_json(nlohmann::json& j, const DynaValue& d);

// --- JSON Deserialization ---
void from_json(const nlohmann::json& j, ConfigValue& val);
void from_json(const nlohmann::json& j, Array& a);
void from_json(const nlohmann::json& j, Set& s);
void from_json(const nlohmann::json& j, Map& m);
void from_json(const nlohmann::json& j, CrossSectionRef& r);
void from_json(const nlohmann::json& j, Color& c);
void from_json(const nlohmann::json& j, Coord& co);
void from_json(const nlohmann::json& j, Path& p);
void from_json(const nlohmann::json& j, DynaValue& d);

// --- YINI String Serialization ---
std::string to_yini_string(const ConfigValue& val);

#endif // PARSER_H