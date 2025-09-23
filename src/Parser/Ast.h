#pragma once

#include <string>
#include <vector>
#include <map>
#include <variant>

// Forward declaration for recursive array type
struct YiniValue;

// --- Value Types ---

using YiniArray = std::vector<YiniValue>;

struct YiniCoord
{
    double x = 0.0, y = 0.0, z = 0.0;
    bool is_3d = false;

    bool operator==(const YiniCoord& other) const
    {
        return x == other.x && y == other.y && z == other.z && is_3d == other.is_3d;
    }
};

struct YiniColor
{
    int r = 0, g = 0, b = 0;

    bool operator==(const YiniColor& other) const
    {
        return r == other.r && g == other.g && b == other.b;
    }
};

// A key-value pair, e.g., {key: value}
using YiniObject = std::map<std::string, YiniValue>;

// A map of key-value pairs is represented by a YiniArray of YiniObjects.
// using YiniMap = std::vector<YiniObject>;


struct YiniMacroRef {
    std::string name;
    bool operator==(const YiniMacroRef& other) const { return name == other.name; }
};

// The main variant type for any value in YINI
using YiniValueType = std::variant<
    std::string,
    int64_t,
    double,
    bool,
    YiniArray,
    YiniCoord,
    YiniColor,
    YiniObject,
    // YiniMap is now handled by YiniArray
    YiniMacroRef
>;

struct YiniValue
{
    YiniValueType value;

    bool operator==(const YiniValue& other) const
    {
        return value == other.value;
    }
};


// --- AST Node Types ---

class YiniSection
{
public:
    std::string name;
    std::vector<std::string> inherits;
    std::map<std::string, YiniValue> keyValues;
    std::vector<YiniValue> autoIndexedValues;

    bool operator==(const YiniSection& other) const
    {
        return name == other.name &&
               inherits == other.inherits &&
               keyValues == other.keyValues &&
               autoIndexedValues == other.autoIndexedValues;
    }
};

class YiniFile
{
public:
    std::map<std::string, YiniValue> definesMap;
    std::vector<std::string> includePaths;
    std::map<std::string, YiniSection> sectionsMap;

    bool operator==(const YiniFile& other) const
    {
        return definesMap == other.definesMap &&
               includePaths == other.includePaths &&
               sectionsMap == other.sectionsMap;
    }
};
