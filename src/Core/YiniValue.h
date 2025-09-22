#pragma once

#include <string>
#include <vector>
#include <map>
#include <variant>
#include <cstdint>

namespace Yini
{
    // Forward declaration for recursive types
    class YiniValue;

    // Basic types
    using YiniInteger = int64_t;
    using YiniFloat = double;
    using YiniBoolean = bool;
    using YiniString = std::string;

    // Complex types
    struct Coordinate2D { float x, y; };
    struct Coordinate3D { float x, y, z; };

    struct ColorRGB { uint8_t r, g, b; };
    struct ColorRGBA { uint8_t r, g, b, a; }; // Though not in spec, good to have

    using YiniArray = std::vector<YiniValue>;
    using YiniMap = std::map<YiniString, YiniValue>;


    // The main variant type
    using YiniVariant = std::variant<
        YiniInteger,
        YiniFloat,
        YiniBoolean,
        YiniString,
        Coordinate2D,
        Coordinate3D,
        ColorRGB,
        ColorRGBA,
        YiniArray,
        YiniMap
    >;

    class YiniValue
    {
    public:
        YiniValue() = default;
        YiniValue(const YiniVariant& value);

        template<typename T>
        T& get()
        {
            return std::get<T>(m_value);
        }

        const YiniVariant& getVariant() const { return m_value; }

        template<typename T>
        const T& get() const
        {
            return std::get<T>(m_value);
        }

        template<typename T>
        void set(const T& value)
        {
            m_value = value;
        }

    private:
        YiniVariant m_value;
    };
}
