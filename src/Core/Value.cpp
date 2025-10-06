#include "Value.h"
#include <stdexcept>

namespace yini
{

Value::Value() : data_(std::monostate{}), is_dynamic_(false) {}

Value::Value(long long val) : data_(val), is_dynamic_(false) {}

Value::Value(double val) : data_(val), is_dynamic_(false) {}

Value::Value(bool val) : data_(val), is_dynamic_(false) {}

Value::Value(const std::string& val) : data_(val), is_dynamic_(false) {}

Value::Value(const Color& val) : data_(val), is_dynamic_(false) {}

Value::Value(const Coord& val) : data_(val), is_dynamic_(false) {}

Value::Value(const Path& val) : data_(val), is_dynamic_(false) {}

ValueType Value::getType() const
{
    if (std::holds_alternative<std::monostate>(data_))
        return ValueType::Null;
    if (std::holds_alternative<long long>(data_))
        return ValueType::Integer;
    if (std::holds_alternative<double>(data_))
        return ValueType::Float;
    if (std::holds_alternative<bool>(data_))
        return ValueType::Boolean;
    if (std::holds_alternative<std::string>(data_))
        return ValueType::String;
    if (std::holds_alternative<std::vector<std::shared_ptr<Value>>>(data_))
        return ValueType::Array;
    if (std::holds_alternative<std::map<std::string, std::shared_ptr<Value>>>(data_))
        return ValueType::Map;
    if (std::holds_alternative<Color>(data_))
        return ValueType::Color;
    if (std::holds_alternative<Coord>(data_))
        return ValueType::Coord;
    if (std::holds_alternative<Path>(data_))
        return ValueType::Path;
    
    return ValueType::Null;
}

bool Value::isNull() const { return getType() == ValueType::Null; }
bool Value::isInteger() const { return getType() == ValueType::Integer; }
bool Value::isFloat() const { return getType() == ValueType::Float; }
bool Value::isBoolean() const { return getType() == ValueType::Boolean; }
bool Value::isString() const { return getType() == ValueType::String; }
bool Value::isArray() const { return getType() == ValueType::Array; }
bool Value::isMap() const { return getType() == ValueType::Map; }
bool Value::isColor() const { return getType() == ValueType::Color; }
bool Value::isCoord() const { return getType() == ValueType::Coord; }
bool Value::isPath() const { return getType() == ValueType::Path; }

long long Value::asInteger() const
{
    if (!isInteger())
        throw std::runtime_error("Value is not an integer");
    return std::get<long long>(data_);
}

double Value::asFloat() const
{
    if (!isFloat())
        throw std::runtime_error("Value is not a float");
    return std::get<double>(data_);
}

bool Value::asBoolean() const
{
    if (!isBoolean())
        throw std::runtime_error("Value is not a boolean");
    return std::get<bool>(data_);
}

std::string Value::asString() const
{
    if (!isString())
        throw std::runtime_error("Value is not a string");
    return std::get<std::string>(data_);
}

Color Value::asColor() const
{
    if (!isColor())
        throw std::runtime_error("Value is not a color");
    return std::get<Color>(data_);
}

Coord Value::asCoord() const
{
    if (!isCoord())
        throw std::runtime_error("Value is not a coord");
    return std::get<Coord>(data_);
}

Path Value::asPath() const
{
    if (!isPath())
        throw std::runtime_error("Value is not a path");
    return std::get<Path>(data_);
}

} // namespace yini
