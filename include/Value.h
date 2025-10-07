#ifndef YINI_VALUE_H
#define YINI_VALUE_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <variant>
#include <optional>

namespace yini
{

// Forward declaration
class Value;

// Value types
enum class ValueType
{
    NIL,
    INTEGER,
    FLOAT,
    BOOLEAN,
    STRING,
    ARRAY,
    LIST,
    MAP,
    TUPLE,
    SET,
    COLOR,
    COORD,
    PATH,
    DYNAMIC,      // Dyna() wrapped value
    REFERENCE,    // @name or @{section.key}
    ENV_VAR       // ${NAME}
};

// Color structure
struct Color
{
    uint8_t r, g, b;
    std::optional<uint8_t> a;
    
    Color(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, std::optional<uint8_t> a = std::nullopt)
        : r(r), g(g), b(b), a(a)
    {
    }
    
    std::string toString() const;
};

// Coordinate structure
struct Coord
{
    double x, y;
    std::optional<double> z;
    
    Coord(double x = 0, double y = 0, std::optional<double> z = std::nullopt)
        : x(x), y(y), z(z)
    {
    }
    
    std::string toString() const;
};

// Value class using strategy pattern for different types
class Value
{
public:
    using ArrayType = std::vector<std::shared_ptr<Value>>;
    using MapType = std::map<std::string, std::shared_ptr<Value>>;
    
    // Constructors
    Value();
    explicit Value(int64_t val);
    explicit Value(double val);
    explicit Value(bool val);
    explicit Value(const std::string& val);
    explicit Value(const char* val);
    explicit Value(const Color& val);
    explicit Value(const Coord& val);
    explicit Value(const ArrayType& val);
    explicit Value(const MapType& val);
    
    // Type checking
    ValueType getType() const { return type; }
    bool isNil() const { return type == ValueType::NIL; }
    bool isInteger() const { return type == ValueType::INTEGER; }
    bool isFloat() const { return type == ValueType::FLOAT; }
    bool isBoolean() const { return type == ValueType::BOOLEAN; }
    bool isString() const { return type == ValueType::STRING; }
    bool isArray() const { return type == ValueType::ARRAY; }
    bool isList() const { return type == ValueType::LIST; }
    bool isMap() const { return type == ValueType::MAP; }
    bool isColor() const { return type == ValueType::COLOR; }
    bool isCoord() const { return type == ValueType::COORD; }
    bool isDynamic() const { return type == ValueType::DYNAMIC; }
    bool isReference() const { return type == ValueType::REFERENCE; }
    bool isEnvVar() const { return type == ValueType::ENV_VAR; }
    
    // Value getters (may throw if type mismatch)
    int64_t asInteger() const;
    double asFloat() const;
    bool asBoolean() const;
    std::string asString() const;
    ArrayType asArray() const;
    MapType asMap() const;
    Color asColor() const;
    Coord asCoord() const;
    
    // Safe value getters (returns optional)
    std::optional<int64_t> tryAsInteger() const;
    std::optional<double> tryAsFloat() const;
    std::optional<bool> tryAsBoolean() const;
    std::optional<std::string> tryAsString() const;
    std::optional<ArrayType> tryAsArray() const;
    std::optional<MapType> tryAsMap() const;
    std::optional<Color> tryAsColor() const;
    std::optional<Coord> tryAsCoord() const;
    
    // Value getters with default (never throws)
    int64_t asIntegerOr(int64_t default_val) const;
    double asFloatOr(double default_val) const;
    bool asBooleanOr(bool default_val) const;
    std::string asStringOr(const std::string& default_val) const;
    
    // String representation
    std::string toString() const;
    
    // Dynamic wrapper
    static std::shared_ptr<Value> makeDynamic(std::shared_ptr<Value> inner);
    
    // Reference value
    static std::shared_ptr<Value> makeReference(const std::string& ref);
    
    // Environment variable
    static std::shared_ptr<Value> makeEnvVar(const std::string& var_name);
    
private:
    ValueType type;
    
    // Storage using variant
    std::variant<
        std::monostate,
        int64_t,
        double,
        bool,
        std::string,
        ArrayType,
        MapType,
        Color,
        Coord,
        std::shared_ptr<Value>  // For dynamic/reference values
    > data;
};

} // namespace yini

#endif // YINI_VALUE_H
