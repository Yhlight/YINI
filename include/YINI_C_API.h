#ifndef YINI_C_API_H
#define YINI_C_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

// Platform-specific export/import macros
#ifdef _WIN32
    #ifdef YINI_EXPORTS
        #define YINI_API __declspec(dllexport)
    #else
        #define YINI_API __declspec(dllimport)
    #endif
#else
    #define YINI_API __attribute__((visibility("default")))
#endif

// Opaque handle types
typedef void* YiniParserHandle;
typedef void* YiniSectionHandle;
typedef void* YiniValueHandle;

// Error codes
typedef enum
{
    YINI_SUCCESS = 0,
    YINI_ERROR_INVALID_HANDLE = -1,
    YINI_ERROR_PARSE_FAILED = -2,
    YINI_ERROR_FILE_NOT_FOUND = -3,
    YINI_ERROR_INVALID_TYPE = -4,
    YINI_ERROR_KEY_NOT_FOUND = -5,
    YINI_ERROR_SECTION_NOT_FOUND = -6
} YiniError;

// Value types
typedef enum
{
    YINI_TYPE_NIL = 0,
    YINI_TYPE_INTEGER = 1,
    YINI_TYPE_FLOAT = 2,
    YINI_TYPE_BOOLEAN = 3,
    YINI_TYPE_STRING = 4,
    YINI_TYPE_ARRAY = 5,
    YINI_TYPE_MAP = 6,
    YINI_TYPE_COLOR = 7,
    YINI_TYPE_COORD = 8
} YiniValueType;

// ============================================================================
// Parser functions
// ============================================================================

/// Create a parser from a YINI source string
/// @param source The YINI source code
/// @return Parser handle (must be freed with yini_parser_destroy)
YINI_API YiniParserHandle yini_parser_create(const char* source);

/// Create a parser from a YINI file
/// @param filename Path to the YINI file
/// @return Parser handle (must be freed with yini_parser_destroy)
YINI_API YiniParserHandle yini_parser_create_from_file(const char* filename);

/// Destroy a parser and free all associated memory
/// @param parser Parser handle to destroy
YINI_API void yini_parser_destroy(YiniParserHandle parser);

/// Parse the YINI source
/// @param parser Parser handle
/// @return true if parsing succeeded, false otherwise
YINI_API bool yini_parser_parse(YiniParserHandle parser);

/// Get the last error message
/// @param parser Parser handle
/// @return Error message (owned by parser, do not free)
YINI_API const char* yini_parser_get_error(YiniParserHandle parser);

// ============================================================================
// Section functions
// ============================================================================

/// Get the number of sections
/// @param parser Parser handle
/// @return Number of sections
YINI_API int yini_parser_get_section_count(YiniParserHandle parser);

/// Get all section names
/// @param parser Parser handle
/// @param count Output parameter for array size
/// @return Array of section names (MUST be freed with yini_free_string_array)
/// @warning CRITICAL: Caller MUST call yini_free_string_array(result, *count)
YINI_API const char** yini_parser_get_section_names(YiniParserHandle parser, int* count);

/// Get a section by name
/// @param parser Parser handle
/// @param name Section name
/// @return Section handle (owned by parser, do not free)
YINI_API YiniSectionHandle yini_parser_get_section(YiniParserHandle parser, const char* name);

/// Get the number of entries in a section
/// @param section Section handle
/// @return Number of entries
YINI_API int yini_section_get_entry_count(YiniSectionHandle section);

/// Get all keys in a section
/// @param section Section handle
/// @param count Output parameter for array size
/// @return Array of keys (MUST be freed with yini_free_string_array)
/// @warning CRITICAL: Caller MUST call yini_free_string_array(result, *count)
YINI_API const char** yini_section_get_keys(YiniSectionHandle section, int* count);

// ============================================================================
// Value functions
// ============================================================================

/// Get a value from a section by key
/// @param section Section handle
/// @param key The key name
/// @return Value handle (owned by section, do not free)
YINI_API YiniValueHandle yini_section_get_value(YiniSectionHandle section, const char* key);

/// Get the type of a value
/// @param value Value handle
/// @return The value type
YINI_API YiniValueType yini_value_get_type(YiniValueHandle value);

// ============================================================================
// Type-specific getters
// ============================================================================

/// Get integer value
/// @param value Value handle
/// @return Integer value (or 0 if type mismatch)
YINI_API int64_t yini_value_get_integer(YiniValueHandle value);

/// Get float value
/// @param value Value handle
/// @return Float value (or 0.0 if type mismatch)
YINI_API double yini_value_get_float(YiniValueHandle value);

/// Get boolean value
/// @param value Value handle
/// @return Boolean value (or false if type mismatch)
YINI_API bool yini_value_get_boolean(YiniValueHandle value);

/// Get string value
/// @param value Value handle
/// @return String value (owned by value, do not free)
YINI_API const char* yini_value_get_string(YiniValueHandle value);

// ============================================================================
// Array functions
// ============================================================================

/// Get the size of an array
/// @param value Value handle (must be array type)
/// @return Array size
YINI_API int yini_value_get_array_size(YiniValueHandle value);

/// Get an element from an array
/// @param value Value handle (must be array type)
/// @param index Element index
/// @return Element value handle (owned by array, do not free)
YINI_API YiniValueHandle yini_value_get_array_element(YiniValueHandle value, int index);

// ============================================================================
// Memory management
// ============================================================================

/// Free a string array allocated by YINI C API
/// @param array The string array to free
/// @param count The number of strings in the array
/// @warning CRITICAL: MUST be called for arrays returned by:
///          - yini_parser_get_section_names()
///          - yini_section_get_keys()
/// @note Failure to call this function will cause memory leaks
YINI_API void yini_free_string_array(const char** array, int count);

// YMETA functions
YINI_API bool yini_compile_to_ymeta(const char* input_file, const char* output_file);
YINI_API bool yini_decompile_from_ymeta(const char* input_file, const char* output_file);

#ifdef __cplusplus
}
#endif

#endif // YINI_C_API_H
