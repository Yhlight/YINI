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

// Parser functions
YINI_API YiniParserHandle yini_parser_create(const char* source);
YINI_API YiniParserHandle yini_parser_create_from_file(const char* filename);
YINI_API void yini_parser_destroy(YiniParserHandle parser);
YINI_API bool yini_parser_parse(YiniParserHandle parser);
YINI_API const char* yini_parser_get_error(YiniParserHandle parser);

// Section functions
YINI_API int yini_parser_get_section_count(YiniParserHandle parser);
YINI_API const char** yini_parser_get_section_names(YiniParserHandle parser, int* count);
YINI_API YiniSectionHandle yini_parser_get_section(YiniParserHandle parser, const char* name);
YINI_API int yini_section_get_entry_count(YiniSectionHandle section);
YINI_API const char** yini_section_get_keys(YiniSectionHandle section, int* count);

// Value functions
YINI_API YiniValueHandle yini_section_get_value(YiniSectionHandle section, const char* key);
YINI_API YiniValueType yini_value_get_type(YiniValueHandle value);

// Type-specific getters
YINI_API int64_t yini_value_get_integer(YiniValueHandle value);
YINI_API double yini_value_get_float(YiniValueHandle value);
YINI_API bool yini_value_get_boolean(YiniValueHandle value);
YINI_API const char* yini_value_get_string(YiniValueHandle value);

// Array functions
YINI_API int yini_value_get_array_size(YiniValueHandle value);
YINI_API YiniValueHandle yini_value_get_array_element(YiniValueHandle value, int index);

// Memory management
YINI_API void yini_free_string_array(const char** array, int count);

// YMETA functions
YINI_API bool yini_compile_to_ymeta(const char* input_file, const char* output_file);
YINI_API bool yini_decompile_from_ymeta(const char* input_file, const char* output_file);

#ifdef __cplusplus
}
#endif

#endif // YINI_C_API_H
