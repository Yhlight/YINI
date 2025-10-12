#pragma once

#include <cstdint>
#include <variant>
#include <vector>
#include <string>
#include <memory>
#include <ostream>

namespace YINI
{

struct ResolvedColor
{
    uint8_t r, g, b;
};

struct ResolvedCoord
{
    double x = 0.0, y = 0.0, z = 0.0;
    bool has_z = false;
};

// Forward declaration for recursion
struct YiniVariant;

// Using an alias for the vector type, which contains the variant type
using YiniArray = std::vector<YiniVariant>;

// The base variant type, using a pointer-wrapper for the recursive part
using YiniVariantBase = std::variant<
    std::monostate, // Represents a null or uninitialized value
    int64_t,
    double,
    bool,
    std::string,
    ResolvedColor,
    ResolvedCoord,
    std::unique_ptr<YiniArray>
>;

// The actual variant type we will use, which inherits from the base.
// This is a common pattern for defining recursive variants.
struct YiniVariant : YiniVariantBase
{
    using YiniVariantBase::YiniVariantBase;
    using YiniVariantBase::operator=;

    // Custom copy constructor for deep copying the unique_ptr
    YiniVariant(const YiniVariant& other) : YiniVariantBase()
    {
        std::visit([&](const auto& value) {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, std::unique_ptr<YiniArray>>) {
                if (value) {
                    *this = std::make_unique<YiniArray>(*value);
                } else {
                    *this = std::unique_ptr<YiniArray>(nullptr);
                }
            } else {
                *this = value;
            }
        }, static_cast<const YiniVariantBase&>(other));
    }

    // Custom copy assignment operator
    YiniVariant& operator=(const YiniVariant& other)
    {
        if (this != &other) {
            std::visit([&](const auto& value) {
                using T = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<T, std::unique_ptr<YiniArray>>) {
                    if (value) {
                        *this = std::make_unique<YiniArray>(*value);
                    } else {
                        *this = std::unique_ptr<YiniArray>(nullptr);
                    }
                } else {
                    *this = value;
                }
            }, static_cast<const YiniVariantBase&>(other));
        }
        return *this;
    }

    // Explicitly default the move constructor and move assignment operator
    YiniVariant(YiniVariant&& other) = default;
    YiniVariant& operator=(YiniVariant&& other) = default;
};

inline std::ostream& operator<<(std::ostream& os, const ResolvedColor& color)
{
    os << "color(" << static_cast<int>(color.r) << ", " << static_cast<int>(color.g) << ", " << static_cast<int>(color.b) << ")";
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const ResolvedCoord& coord)
{
    os << "coord(" << coord.x << ", " << coord.y;
    if (coord.has_z) {
        os << ", " << coord.z;
    }
    os << ")";
    return os;
}

} // namespace YINI
