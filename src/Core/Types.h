#ifndef YINI_CORE_TYPES_H
#define YINI_CORE_TYPES_H

#include <string>
#include <vector>
#include <map>
#include <variant>
#include <memory>
#include <optional>

namespace yini
{

// Forward declarations
class Value;

// Type enumeration
enum class ValueType
{
    Null,
    Integer,
    Float,
    Boolean,
    String,
    Array,
    List,
    Set,
    Pair,
    Map,
    Color,
    Coord,
    Path,
    Dynamic
};

// Color type (RGB)
struct Color
{
    int r;
    int g;
    int b;
    
    Color() : r(0), g(0), b(0) {}
    Color(int red, int green, int blue) : r(red), g(green), b(blue) {}
};

// Coordinate type (2D or 3D)
struct Coord
{
    double x;
    double y;
    std::optional<double> z;
    
    Coord() : x(0), y(0), z(std::nullopt) {}
    Coord(double x_val, double y_val) : x(x_val), y(y_val), z(std::nullopt) {}
    Coord(double x_val, double y_val, double z_val) : x(x_val), y(y_val), z(z_val) {}
};

// Path type
struct Path
{
    std::string path;
    
    Path() : path("") {}
    Path(const std::string& p) : path(p) {}
};

// Convert ValueType to string
std::string value_type_to_string(ValueType type);

// Parse color from hex string
std::optional<Color> parse_hex_color(const std::string& hex);

} // namespace yini

#endif // YINI_CORE_TYPES_H
