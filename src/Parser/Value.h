#ifndef YINI_VALUE_H
#define YINI_VALUE_H

#include <string>
#include <vector>
#include <variant>
#include <cstdint>
#include <memory>

namespace YINI
{

// Forward declarations
struct Array;
struct Coordinate;
struct Color;

// The main Value type
using Value = std::variant<
    std::monostate, // For empty/null values
    std::string,
    long long,
    double,
    bool,
    std::unique_ptr<Array>,
    std::unique_ptr<Coordinate>,
    std::unique_ptr<Color>
>;

struct Array
{
    std::vector<Value> elements;
};

struct Coordinate
{
    double x = 0.0;
    double y = 0.0;
};

struct Color
{
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 255;
};

} // namespace YINI

#endif // YINI_VALUE_H
