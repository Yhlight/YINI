#pragma once

#include "Ast.h"
#include <string>
#include <cstdint>

// Defines the binary format for .ymeta files
namespace Ymeta
{
    // Tags to identify data types in the binary file
    enum class MetaTag : uint8_t
    {
        // Value Types
        NULL_VALUE,
        STRING,
        INT64,
        DOUBLE,
        BOOL_TRUE,
        BOOL_FALSE,
        ARRAY,
        COORD,
        COLOR,
        OBJECT,
        MAP,
        MACRO_REF, // Added for completeness, though should not be serialized

        // Structural Tags
        YINI_FILE_START,
        DEFINES_START,
        INCLUDES_START,
        SECTIONS_START,
        SECTION_START,
        INHERITS_START,
        KEY_VALUES_START,
        AUTO_INDEXED_START,
        YINI_FILE_END,
    };

    class Serializer
    {
    public:
        void serialize(const YiniFile& ast, const std::string& filepath);
    };

    class Deserializer
    {
    public:
        YiniFile deserialize(const std::string& filepath);
    };

} // namespace Ymeta
