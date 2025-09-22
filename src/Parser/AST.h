#pragma once

#include <string>
#include <vector>
#include <variant>
#include <map>
#include <memory>

namespace YINI
{
    // Forward declarations
    struct Value;
    struct Section;

    // Value types
    using String = std::string;
    using Integer = long long;
    using Float = double;
    using Boolean = bool;

    struct Coordinate { double x, y, z; bool has_z; bool operator==(const Coordinate& other) const { return x == other.x && y == other.y && z == other.z && has_z == other.has_z; } };
    struct Color { int r, g, b; bool operator==(const Color& other) const { return r == other.r && g == other.g && b == other.b; } };

    using Array = std::vector<std::unique_ptr<Value>>;
    using Map = std::map<std::string, std::unique_ptr<Value>>;
    struct Macro { std::string name; bool operator==(const Macro& other) const { return name == other.name; } };

    // The main Value variant
    struct Value
    {
        std::variant<
            String,
            Integer,
            Float,
            Boolean,
            Coordinate,
            Color,
            Array,
            Map,
            Macro
        > data;
    };

    bool operator==(const Value& lhs, const Value& rhs);

    // Key-value pair in a section
    struct KeyValuePair
    {
        std::string key;
        std::unique_ptr<Value> value;
        bool is_quick_registration; // true if += was used

        KeyValuePair() = default;
        KeyValuePair(std::string k, std::unique_ptr<Value> v, bool is_qr)
            : key(std::move(k)), value(std::move(v)), is_quick_registration(is_qr) {}
    };

    // Section
    struct Section
    {
        std::string name;
        std::vector<std::string> inherits;
        std::vector<KeyValuePair> pairs;

        bool is_define_section = false;
        bool is_include_section = false;
    };

    // The root of the YINI file representation
    struct YiniFile
    {
        std::map<std::string, Section> sections;
    };
}
