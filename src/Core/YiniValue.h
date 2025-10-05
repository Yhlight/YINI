/**
 * @file YiniValue.h
 * @brief Defines the YiniValue class, a type-safe variant for holding all supported YINI data types.
 */
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

    /**
     * @typedef YiniArray
     * @brief A type alias for a vector of YiniValue objects, representing an array in YINI.
     */
    using YiniArray = std::vector<YiniValue>;

    /**
     * @typedef YiniMap
     * @brief A type alias for a map with string keys and YiniValue values, representing a map in YINI.
     */
    using YiniMap = std::map<std::string, YiniValue, std::less<>>;

    /**
     * @typedef YiniValueBase
     * @brief The underlying std::variant that holds the possible YINI data types.
     *
     * `std::unique_ptr` is used for recursive types (arrays, maps, and dynamic values)
     * to break the circular dependency and allow for incomplete types in the variant.
     */
    using YiniValueBase = std::variant<
        std::monostate, // Represents a null or uninitialized value
        bool,
        double,
        std::string,
        std::unique_ptr<YiniArray>,
        std::unique_ptr<YiniMap>,
        std::unique_ptr<DynaValue>
    >;

    /**
     * @class YiniValue
     * @brief A wrapper around a std::variant to provide a type-safe container for any value that can be represented in YINI.
     *
     * This class provides constructors for easy creation of values and manages the
     * memory for heap-allocated recursive types like arrays and maps.
     */
    class YiniValue
    {
    public:
        // Constructors
        YiniValue();
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

        /**
         * @brief The underlying std::variant holding the actual value.
         */
        YiniValueBase m_value;

        /**
         * @brief Returns the name of the type currently held by the YiniValue.
         * @return A string representing the type name (e.g., "string", "number", "array").
         */
        std::string get_type_name() const;
    };
}