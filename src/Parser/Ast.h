#ifndef YINI_AST_H
#define YINI_AST_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <variant>
#include <cstdint>
#include <optional>

namespace YINI
{
    // Forward declaration for the variant-like value type
    struct YiniValue;

    // Represents a key-value pair
    struct KeyValueNode
    {
        std::string key;
        std::unique_ptr<YiniValue> value;
    };

    // Represents a section (e.g., [SectionName])
    struct SectionNode
    {
        std::string name;
        std::vector<std::string> parents;
        std::vector<KeyValueNode> key_values;
    };

    // Represents the root of a YINI document
    struct AstNode
    {
        std::map<std::string, std::unique_ptr<YiniValue>> macros;
        std::vector<SectionNode> sections;
    };

    // A variant-like class to hold different value types
    struct Color {
        uint8_t r, g, b;
        bool operator==(const Color& other) const {
            return r == other.r && g == other.g && b == other.b;
        }
    };

    struct CrossSectionRef {
        std::string section;
        std::string key;
        bool operator==(const CrossSectionRef& other) const {
            return section == other.section && key == other.key;
        }
    };

    struct Path {
        std::string value;
        bool operator==(const Path& other) const {
            return value == other.value;
        }
    };

    struct Coord {
        double x, y;
        std::optional<double> z;
        bool operator==(const Coord& other) const {
            return x == other.x && y == other.y && z == other.z;
        }
    };

    struct YiniValue
    {
        // Forward declare the recursive types
        using Array = std::vector<YiniValue>;
        using Map = std::map<std::string, YiniValue>;
        using DynaWrapper = std::unique_ptr<YiniValue>;

        // We will expand this to support arrays and maps later.
        // For recursive types, we'll need std::unique_ptr wrappers.
        using ValueVariant = std::variant<
            std::string,
            int64_t,
            double,
            bool,
            std::unique_ptr<Array>,
            std::unique_ptr<Map>,
            Color,
            Coord,
            Path,
            CrossSectionRef,
            DynaWrapper
        >;

        ValueVariant value;

        // Explicitly define move semantics and delete copy semantics to handle std::unique_ptr
        YiniValue() = default;
        YiniValue(YiniValue&&) = default;
        YiniValue& operator=(YiniValue&&) = default;
        YiniValue(const YiniValue&) = delete;
        YiniValue& operator=(const YiniValue&) = delete;

        std::unique_ptr<YiniValue> clone() const;
    };
}

#endif // YINI_AST_H