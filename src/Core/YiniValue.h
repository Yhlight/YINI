#pragma once

#include <variant>
#include <string>
#include <vector>
#include <map>
#include <memory>

namespace YINI
{
    // Forward-declare recursive types
    class YiniValue;
    class DynaValue;

    // Define collection types using YiniValue
    using YiniArray = std::vector<YiniValue>;
    using YiniMap = std::map<std::string, YiniValue>;

    // Use unique_ptr to break the recursive dependency for std::variant
    using YiniValueBase = std::variant<
        std::monostate, // Represents a null or uninitialized value
        bool,
        double,
        std::string,
        std::unique_ptr<YiniArray>,
        std::unique_ptr<YiniMap>,
        std::unique_ptr<DynaValue>
    >;

    // YiniValue is a wrapper around the variant to provide a nicer interface
    // and handle the unique_ptr indirection.
    class YiniValue
    {
    public:
        // Constructors
        YiniValue(); // default to monostate
        YiniValue(bool value);
        YiniValue(double value);
        YiniValue(const std::string& value);
        YiniValue(const char* value);
        YiniValue(YiniArray&& value);
        YiniValue(const YiniArray& value);
        YiniValue(YiniMap&& value);
        YiniValue(const YiniMap& value);
        YiniValue(DynaValue&& value);
        YiniValue(const DynaValue& value);

        // Destructor, copy/move constructors and assignments must be defined in the .cpp file
        // because of the unique_ptr to an incomplete type.
        ~YiniValue();
        YiniValue(const YiniValue& other);
        YiniValue& operator=(const YiniValue& other);
        YiniValue(YiniValue&& other) noexcept;
        YiniValue& operator=(YiniValue&& other) noexcept;

        // The underlying variant
        YiniValueBase m_value;
    };
}