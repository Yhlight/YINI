/**
 * @file YiniValue.h
 * @brief Defines the YiniValue class, a type-safe variant for holding all supported YINI data types.
 * @ingroup Core
 */
#pragma once

#include <variant>
#include <string>
#include <vector>
#include <map>
#include <memory>

namespace YINI
{
    // Forward-declare recursive types to be used in the variant
    class YiniValue;
    class DynaValue;

    /**
     * @typedef YiniArray
     * @brief A type alias for a vector of YiniValue objects. Represents an array in YINI.
     * @ingroup Core
     */
    using YiniArray = std::vector<YiniValue>;

    /**
     * @typedef YiniMap
     * @brief A type alias for a map with string keys and YiniValue values. Represents a map in YINI.
     * @ingroup Core
     */
    using YiniMap = std::map<std::string, YiniValue>;

    /**
     * @typedef YiniValueBase
     * @brief The underlying `std::variant` that holds all possible YINI data types.
     * @ingroup Core
     *
     * @details `std::unique_ptr` is used for recursive types (YiniArray, YiniMap, and DynaValue)
     * to break the circular dependency that would otherwise prevent compilation. This allows
     * for heap allocation of nested structures.
     */
    using YiniValueBase = std::variant<
        std::monostate, ///< Represents a null or uninitialized value.
        bool,           ///< Represents a boolean value (`true` or `false`).
        double,         ///< Represents a floating-point number.
        std::string,    ///< Represents a string of text.
        std::unique_ptr<YiniArray>,  ///< Represents a list of other YiniValue objects.
        std::unique_ptr<YiniMap>,    ///< Represents a key-value mapping.
        std::unique_ptr<DynaValue>   ///< Represents a dynamically updatable value.
    >;

    /**
     * @class YiniValue
     * @brief A RAII wrapper around a `std::variant` to provide a type-safe container for any value in YINI.
     * @ingroup Core
     *
     * @details This class simplifies the creation and management of YINI values. It provides
     * convenient constructors for all supported types and correctly handles the memory
     * management for heap-allocated recursive types like arrays and maps via `std::unique_ptr`.
     *
     * The special member functions (destructor, copy/move constructors, and assignments) are
     * explicitly declared here and defined in the `.cpp` file. This is necessary because the
     * `std::unique_ptr` members point to incomplete types at this stage.
     */
    class YiniValue
    {
    public:
        /// @name Constructors
        /// @{

        /// @brief Default constructor. Initializes the value to `std::monostate` (null).
        YiniValue();
        /// @brief Constructs a YiniValue from a boolean.
        YiniValue(bool value);
        /// @brief Constructs a YiniValue from a double.
        YiniValue(double value);
        /// @brief Constructs a YiniValue from a C++ string.
        YiniValue(const std::string& value);
        /// @brief Constructs a YiniValue from a C-style string literal.
        YiniValue(const char* value);
        /// @brief Constructs a YiniValue by moving a YiniArray.
        YiniValue(YiniArray&& value);
        /// @brief Constructs a YiniValue by copying a YiniArray.
        YiniValue(const YiniArray& value);
        /// @brief Constructs a YiniValue by moving a YiniMap.
        YiniValue(YiniMap&& value);
        /// @brief Constructs a YiniValue by copying a YiniMap.
        YiniValue(const YiniMap& value);
        /// @brief Constructs a YiniValue by moving a DynaValue.
        YiniValue(DynaValue&& value);
        /// @brief Constructs a YiniValue by copying a DynaValue.
        YiniValue(const DynaValue& value);

        /// @}

        /// @name Special Member Functions
        /// @{

        /// @brief Destructor.
        ~YiniValue();
        /// @brief Copy constructor.
        YiniValue(const YiniValue& other);
        /// @brief Copy assignment operator.
        YiniValue& operator=(const YiniValue& other);
        /// @brief Move constructor.
        YiniValue(YiniValue&& other) noexcept;
        /// @brief Move assignment operator.
        YiniValue& operator=(YiniValue&& other) noexcept;

        /// @}

        /**
         * @brief The underlying `std::variant` that holds the actual data.
         *
         * @details Direct access is provided for flexibility, but it is recommended to
         * use `std::get`, `std::get_if`, or `std::visit` for type-safe access.
         */
        YiniValueBase m_value;
    };
}