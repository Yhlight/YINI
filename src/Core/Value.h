#ifndef YINI_CORE_VALUE_H
#define YINI_CORE_VALUE_H

#include "Types.h"
#include <memory>
#include <vector>
#include <map>
#include <variant>

namespace yini
{

// Value class using variant for different types
class Value
{
public:
    using ValueVariant = std::variant<
        std::monostate,                              // Null
        long long,                                   // Integer
        double,                                      // Float
        bool,                                        // Boolean
        std::string,                                 // String
        std::vector<std::shared_ptr<Value>>,        // Array/List
        std::map<std::string, std::shared_ptr<Value>>, // Map
        Color,                                       // Color
        Coord,                                       // Coord
        Path                                         // Path
    >;
    
    Value();
    explicit Value(long long val);
    explicit Value(double val);
    explicit Value(bool val);
    explicit Value(const std::string& val);
    explicit Value(const Color& val);
    explicit Value(const Coord& val);
    explicit Value(const Path& val);
    
    ValueType getType() const;
    const ValueVariant& getData() const { return data_; }
    
    // Type checking
    bool isNull() const;
    bool isInteger() const;
    bool isFloat() const;
    bool isBoolean() const;
    bool isString() const;
    bool isArray() const;
    bool isMap() const;
    bool isColor() const;
    bool isCoord() const;
    bool isPath() const;
    
    // Getters
    long long asInteger() const;
    double asFloat() const;
    bool asBoolean() const;
    std::string asString() const;
    Color asColor() const;
    Coord asCoord() const;
    Path asPath() const;
    
    // Dynamic flag
    void setDynamic(bool is_dynamic) { is_dynamic_ = is_dynamic; }
    bool isDynamic() const { return is_dynamic_; }
    
private:
    ValueVariant data_;
    bool is_dynamic_;
};

} // namespace yini

#endif // YINI_CORE_VALUE_H
