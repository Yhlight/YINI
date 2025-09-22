#pragma once

#include <cstdint>

namespace YINI
{
    namespace YMeta
    {
        // File Header
        const char MAGIC[4] = {'Y', 'M', 'E', 'T'};
        const uint8_t VERSION = 1;

        // Type Tags
        enum class Tag : uint8_t
        {
            // Meta
            EndOfFile = 0,
            SectionStart = 1,
            KeyValuePair = 2,

            // Primitives
            String = 10,
            Integer = 11,
            Float = 12,
            Boolean = 13,

            // Complex Types
            Coordinate = 20,
            Color = 21,
            ArrayStart = 22,
            ArrayEnd = 23,
            MapStart = 24,
            MapEnd = 25,
        };
    }
}
