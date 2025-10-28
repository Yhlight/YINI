#include "Types.h"
#include <sstream>
#include <iomanip>

namespace yini
{

std::string value_type_to_string(ValueType type)
{
    switch (type)
    {
        case ValueType::Null:
            return "null";
        case ValueType::Integer:
            return "int";
        case ValueType::Float:
            return "float";
        case ValueType::Boolean:
            return "bool";
        case ValueType::String:
            return "string";
        case ValueType::Array:
            return "array";
        case ValueType::List:
            return "list";
        case ValueType::Set:
            return "set";
        case ValueType::Pair:
            return "pair";
        case ValueType::Map:
            return "map";
        case ValueType::Color:
            return "color";
        case ValueType::Coord:
            return "coord";
        case ValueType::Path:
            return "path";
        case ValueType::Dynamic:
            return "dynamic";
        default:
            return "unknown";
    }
}

std::optional<Color> parse_hex_color(const std::string& hex)
{
    if (hex.length() != 7 || hex[0] != '#')
    {
        return std::nullopt;
    }
    
    try
    {
        int r = std::stoi(hex.substr(1, 2), nullptr, 16);
        int g = std::stoi(hex.substr(3, 2), nullptr, 16);
        int b = std::stoi(hex.substr(5, 2), nullptr, 16);
        
        return Color(r, g, b);
    }
    catch (...)
    {
        return std::nullopt;
    }
}

} // namespace yini
