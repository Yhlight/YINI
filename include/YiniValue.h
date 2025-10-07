#ifndef YINI_VALUE_H
#define YINI_VALUE_H

#include <string>
#include <vector>
#include <list>
#include <map>
#include <variant>
#include <memory>
#include <cstdint>

class YiniValue;

// --- Custom Data Types ---
struct YiniColor {
    uint8_t r, g, b, a;
    bool operator==(const YiniColor& other) const {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }
};

struct YiniCoord {
    double x, y, z;
    bool is_3d;
    bool operator==(const YiniCoord& other) const {
        return x == other.x && y == other.y && z == other.z && is_3d == other.is_3d;
    }
};

struct YiniPath {
    std::string path;
    bool operator==(const YiniPath& other) const {
        return path == other.path;
    }
};


// --- Container Type Aliases ---
using YiniArray = std::vector<YiniValue>;
using YiniList = std::list<YiniValue>;
using YiniMap = std::map<std::string, YiniValue>;

// The variant uses unique_ptr to handle the recursive definition of YiniValue inside containers.
using YiniVariant = std::variant<
    std::string,
    int,
    bool,
    double,
    std::unique_ptr<YiniArray>,
    std::unique_ptr<YiniList>,
    std::unique_ptr<YiniMap>,
    YiniColor,
    YiniCoord,
    YiniPath
>;

class YiniValue {
public:
    // Constructors
    YiniValue();
    YiniValue(const char* value);
    YiniValue(std::string value);
    YiniValue(int value);
    YiniValue(bool value);
    YiniValue(double value);
    YiniValue(YiniArray value);
    YiniValue(YiniList value);
    YiniValue(YiniMap value);
    YiniValue(YiniColor value);
    YiniValue(YiniCoord value);
    YiniValue(YiniPath value);

    // Rule of Five for proper memory management with unique_ptr
    ~YiniValue();
    YiniValue(const YiniValue& other);
    YiniValue(YiniValue&& other) noexcept;
    YiniValue& operator=(const YiniValue& other);
    YiniValue& operator=(YiniValue&& other) noexcept;

    // Type checking and getting values
    template<typename T>
    bool is() const;

    template<typename T>
    const T& get() const;

    template<typename T>
    T& get();

private:
    YiniVariant m_value;
};

#endif // YINI_VALUE_H