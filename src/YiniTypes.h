#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <variant>
#include <vector>

namespace YINI
{

/// @brief Represents a resolved RGB color value.
struct ResolvedColor
{
    uint8_t r, g, b;
};

/// @brief Represents a resolved 2D or 3D coordinate.
struct ResolvedCoord
{
    double x = 0.0, y = 0.0, z = 0.0;
    bool has_z = false;
};

// Forward declaration for recursion
struct YiniVariant;

/// @brief An alias for a vector of YiniVariant objects, representing an array in YINI.
/// @see YiniVariant
using YiniArray = std::vector<YiniVariant>;

/// @brief An alias for a pair representing a single key-value struct.
/// @details The key is a std::string and the value is a unique_ptr to another YiniVariant.
/// @see YiniVariant
using YiniStruct = std::pair<std::string, std::unique_ptr<YiniVariant>>;

/// @brief An alias for a map of string keys to YiniVariant values, representing a map in YINI.
/// @see YiniVariant
using YiniMap = std::map<std::string, YiniVariant>;

/// @brief The base std::variant definition for YiniVariant.
/// @details This defines all possible types that a YiniVariant can hold.
///          The recursive YiniVariant inherits from this.
using YiniVariantBase = std::variant<std::monostate, // Represents a null or uninitialized value
                                     int64_t, double, bool, std::string, ResolvedColor, ResolvedCoord,
                                     YiniMap, YiniStruct, std::unique_ptr<YiniArray>>;

/**
 * @brief The core recursive variant type used to represent any resolved YINI value.
 * @details This is the primary data structure for holding resolved values after parsing and
 *          resolving. It's a std::variant that can hold simple types, YINI-specific structs,
 *          or containers of itself (like arrays). It implements custom copy constructors/assignment
 *          to handle deep copying of heap-allocated members like YiniArray and YiniStruct.
 * @see YiniVariantBase
 */
struct YiniVariant : YiniVariantBase
{
    using YiniVariantBase::YiniVariantBase;
    using YiniVariantBase::operator=;

    /// @brief Custom copy constructor for deep copying move-only types.
    YiniVariant(const YiniVariant &other) : YiniVariantBase()
    {
        std::visit(
            [&](auto &&arg)
            {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, std::unique_ptr<YiniArray>>)
                {
                    *this = arg ? std::make_unique<YiniArray>(*arg) : nullptr;
                }
                else if constexpr (std::is_same_v<T, YiniStruct>)
                {
                    auto new_val = arg.second ? std::make_unique<YiniVariant>(*arg.second) : nullptr;
                    *this = YiniStruct(arg.first, std::move(new_val));
                }
                else if constexpr (std::is_same_v<T, YiniMap>)
                {
                    *this = arg; // std::map is copyable
                }
                else
                {
                    *this = arg;
                }
            },
            static_cast<const YiniVariantBase &>(other));
    }

    /// @brief Custom copy assignment operator for deep copying.
    YiniVariant &operator=(const YiniVariant &other)
    {
        if (this != &other)
        {
            std::visit(
                [&](auto &&arg)
                {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, std::unique_ptr<YiniArray>>)
                    {
                        *this = arg ? std::make_unique<YiniArray>(*arg) : nullptr;
                    }
                    else if constexpr (std::is_same_v<T, YiniStruct>)
                    {
                        auto new_val = arg.second ? std::make_unique<YiniVariant>(*arg.second) : nullptr;
                        *this = YiniStruct(arg.first, std::move(new_val));
                    }
                    else if constexpr (std::is_same_v<T, YiniMap>)
                    {
                        *this = arg; // std::map is copyable
                    }
                    else
                    {
                        *this = arg;
                    }
                },
                static_cast<const YiniVariantBase &>(other));
        }
        return *this;
    }

    // Explicitly default the move constructor and move assignment operator
    YiniVariant(YiniVariant &&other) = default;
    YiniVariant &operator=(YiniVariant &&other) = default;
};

/// @brief ostream operator for pretty-printing ResolvedColor.
inline std::ostream &operator<<(std::ostream &os, const ResolvedColor &color)
{
    os << "color(" << static_cast<int>(color.r) << ", " << static_cast<int>(color.g) << ", "
       << static_cast<int>(color.b) << ")";
    return os;
}

/// @brief ostream operator for pretty-printing ResolvedCoord.
inline std::ostream &operator<<(std::ostream &os, const ResolvedCoord &coord)
{
    os << "coord(" << coord.x << ", " << coord.y;
    if (coord.has_z)
    {
        os << ", " << coord.z;
    }
    os << ")";
    return os;
}

} // namespace YINI
