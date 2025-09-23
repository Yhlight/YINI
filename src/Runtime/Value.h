#ifndef YINI_VALUE_H
#define YINI_VALUE_H

#include <string>
#include <variant>
#include <vector>
#include <map>
#include <memory>
#include <sstream>

namespace Yini
{
    // Forward declaration for recursive types like arrays and maps
    struct Value;

    // --- Value Types ---
    using Integer = long long;
    using Float = double;
    using Boolean = bool;
    using String = std::string;

    struct Coord
    {
        double x = 0, y = 0, z = 0;
        bool is_3d = false;
    };

    struct Color
    {
        uint8_t r = 0, g = 0, b = 0;
    };

    // Using shared_ptr to allow for recursive/shared ownership in complex structures
    using Array = std::vector<std::shared_ptr<Value>>;
    using Map = std::map<String, std::shared_ptr<Value>>;

    // The main Value object using a variant
    struct Value
    {
        std::variant<
            std::monostate, // Represents a null/empty value
            Integer,
            Float,
            Boolean,
            String,
            Coord,
            Color,
            Array,
            Map
        > data;

        // Helper for easy string conversion for debugging
        std::string toString() const;
    };

    // --- Implementation of toString() ---

    // Overload pattern for visiting the variant
    template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
    template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

    inline std::string Value::toString() const
    {
        std::stringstream ss;
        std::visit(overloaded {
            [&](std::monostate) { ss << "null"; },
            [&](Integer i) { ss << i; },
            [&](Float f) { ss << f; },
            [&](Boolean b) { ss << (b ? "true" : "false"); },
            [&](const String& s) { ss << '"' << s << '"'; },
            [&](const Coord& c) {
                ss << "Coord(" << c.x << ", " << c.y;
                if (c.is_3d) ss << ", " << c.z;
                ss << ")";
            },
            [&](const Color& c) {
                ss << "Color(" << (int)c.r << ", " << (int)c.g << ", " << (int)c.b << ")";
            },
            [&](const Array& a) {
                ss << "[";
                for(size_t i = 0; i < a.size(); ++i) {
                    ss << a[i]->toString() << (i == a.size() - 1 ? "" : ", ");
                }
                ss << "]";
            },
            [&](const Map& m) {
                ss << "{";
                size_t i = 0;
                for(const auto& [key, val] : m) {
                    ss << '"' << key << "\": " << val->toString() << (i == m.size() - 1 ? "" : ", ");
                    i++;
                }
                ss << "}";
            }
        }, data);
        return ss.str();
    }
}

#endif // YINI_VALUE_H
