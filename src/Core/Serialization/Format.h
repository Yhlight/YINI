#pragma once

#include <cstdint>

namespace YINI
{
    namespace Serialization
    {
        enum class DataType : uint8_t
        {
            NIL,
            BOOL,
            DOUBLE,
            STRING,
            VECTOR, // Used for both arrays and sets
            MAP
        };
    }
}