#ifndef YINI_VALUE_H
#define YINI_VALUE_H

#include <string>
#include <vector>
#include <map>
#include <variant>
#include <memory>

class YiniValue;

using YiniArray = std::vector<YiniValue>;
using YiniMap = std::map<std::string, YiniValue>;

// The variant uses unique_ptr to handle the recursive definition of YiniValue inside containers.
using YiniVariant = std::variant<
    std::string,
    int,
    bool,
    double,
    std::unique_ptr<YiniArray>,
    std::unique_ptr<YiniMap>
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
    YiniValue(YiniMap value);

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