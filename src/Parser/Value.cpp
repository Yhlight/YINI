#include "Value.h"
#include <sstream>
#include <iomanip>

namespace yini
{

// Color methods

std::string Color::toString() const
{
    std::ostringstream oss;
    oss << "#";
    oss << std::hex << std::setfill('0') << std::uppercase;
    oss << std::setw(2) << static_cast<int>(r);
    oss << std::setw(2) << static_cast<int>(g);
    oss << std::setw(2) << static_cast<int>(b);
    if (a.has_value())
    {
        oss << std::setw(2) << static_cast<int>(a.value());
    }
    return oss.str();
}

// Coord methods

std::string Coord::toString() const
{
    std::ostringstream oss;
    oss << "Coord(" << x << ", " << y;
    if (z.has_value())
    {
        oss << ", " << z.value();
    }
    oss << ")";
    return oss.str();
}

bool Value::isNumeric() const
{
    return type == ValueType::INTEGER || type == ValueType::FLOAT;
}

// Value methods

Value::Value()
    : type(ValueType::NIL)
    , data(std::monostate{})
{
}

Value::Value(int64_t val)
    : type(ValueType::INTEGER)
    , data(val)
{
}

Value::Value(double val)
    : type(ValueType::FLOAT)
    , data(val)
{
}

Value::Value(bool val)
    : type(ValueType::BOOLEAN)
    , data(val)
{
}

Value::Value(const std::string& val)
    : type(ValueType::STRING)
    , data(val)
{
}

Value::Value(const char* val)
    : type(ValueType::STRING)
    , data(std::string(val))
{
}

Value::Value(const Color& val)
    : type(ValueType::COLOR)
    , data(val)
{
}

Value::Value(const Coord& val)
    : type(ValueType::COORD)
    , data(val)
{
}

Value::Value(const ArrayType& val)
    : type(ValueType::ARRAY)
    , data(val)
{
}

Value::Value(const MapType& val)
    : type(ValueType::MAP)
    , data(val)
{
}

int64_t Value::asInteger() const
{
    if (type == ValueType::INTEGER)
    {
        return std::get<int64_t>(data);
    }
    throw std::runtime_error("Value is not an integer");
}

double Value::asFloat() const
{
    if (type == ValueType::FLOAT)
    {
        return std::get<double>(data);
    }
    if (type == ValueType::INTEGER)
    {
        return static_cast<double>(std::get<int64_t>(data));
    }
    throw std::runtime_error("Value is not a float");
}

bool Value::asBoolean() const
{
    if (type == ValueType::BOOLEAN)
    {
        return std::get<bool>(data);
    }
    throw std::runtime_error("Value is not a boolean");
}

std::string Value::asString() const
{
    if (type == ValueType::STRING)
    {
        return std::get<std::string>(data);
    }
    // Also allow REFERENCE and ENV_VAR to return their string content
    if (type == ValueType::REFERENCE || type == ValueType::ENV_VAR)
    {
        return std::get<std::string>(data);
    }
    throw std::runtime_error("Value is not a string");
}

Value::ArrayType Value::asArray() const
{
    if (type == ValueType::ARRAY || type == ValueType::LIST)
    {
        return std::get<ArrayType>(data);
    }
    throw std::runtime_error("Value is not an array");
}

Value::MapType Value::asMap() const
{
    if (type == ValueType::MAP)
    {
        return std::get<MapType>(data);
    }
    throw std::runtime_error("Value is not a map");
}

Color Value::asColor() const
{
    if (type == ValueType::COLOR)
    {
        return std::get<Color>(data);
    }
    throw std::runtime_error("Value is not a color");
}

Coord Value::asCoord() const
{
    if (type == ValueType::COORD)
    {
        return std::get<Coord>(data);
    }
    throw std::runtime_error("Value is not a coord");
}

std::string Value::toString() const
{
    std::ostringstream oss;
    
    switch (type)
    {
        case ValueType::NIL:
            oss << "nil";
            break;
        
        case ValueType::INTEGER:
            oss << std::get<int64_t>(data);
            break;
        
        case ValueType::FLOAT:
            oss << std::get<double>(data);
            break;
        
        case ValueType::BOOLEAN:
            oss << (std::get<bool>(data) ? "true" : "false");
            break;
        
        case ValueType::STRING:
            oss << "\"" << std::get<std::string>(data) << "\"";
            break;
        
        case ValueType::ARRAY:
        case ValueType::LIST:
        {
            auto arr = std::get<ArrayType>(data);
            oss << "[";
            for (size_t i = 0; i < arr.size(); i++)
            {
                if (i > 0) oss << ", ";
                oss << arr[i]->toString();
            }
            oss << "]";
            break;
        }
        
        case ValueType::MAP:
        {
            auto map = std::get<MapType>(data);
            oss << "{";
            bool first = true;
            for (const auto& [key, value] : map)
            {
                if (!first) oss << ", ";
                first = false;
                oss << key << ": " << value->toString();
            }
            oss << "}";
            break;
        }
        
        case ValueType::COLOR:
            oss << std::get<Color>(data).toString();
            break;
        
        case ValueType::COORD:
            oss << std::get<Coord>(data).toString();
            break;
        
        case ValueType::PATH:
            oss << "Path(" << std::get<std::string>(data) << ")";
            break;
        
        case ValueType::DYNAMIC:
        {
            auto inner = std::get<std::shared_ptr<Value>>(data);
            oss << "Dyna(" << inner->toString() << ")";
            break;
        }
        
        case ValueType::REFERENCE:
            oss << "@" << std::get<std::string>(data);
            break;
        
        case ValueType::ENV_VAR:
            oss << "${" << std::get<std::string>(data) << "}";
            break;
        
        default:
            oss << "unknown";
            break;
    }
    
    return oss.str();
}

std::shared_ptr<Value> Value::makeDynamic(std::shared_ptr<Value> inner)
{
    auto val = std::make_shared<Value>();
    val->type = ValueType::DYNAMIC;
    val->data = inner;
    return val;
}

std::shared_ptr<Value> Value::makeReference(const std::string& ref)
{
    auto val = std::make_shared<Value>();
    val->type = ValueType::REFERENCE;
    val->data = ref;
    return val;
}

std::shared_ptr<Value> Value::makeEnvVar(const std::string& var_name)
{
    auto val = std::make_shared<Value>();
    val->type = ValueType::ENV_VAR;
    val->data = var_name;
    return val;
}

std::shared_ptr<Value> Value::makePath(const std::string& path)
{
    auto val = std::make_shared<Value>();
    val->type = ValueType::PATH;
    val->data = path;
    return val;
}

std::shared_ptr<Value> Value::makeList(const ArrayType& elements)
{
    auto val = std::make_shared<Value>();
    val->type = ValueType::LIST;
    val->data = elements;
    return val;
}

} // namespace yini
