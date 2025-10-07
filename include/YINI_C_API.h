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

// --- Opaque Handles ---

// Represents a handle to a fully parsed and interpreted YINI document.
// This is the primary handle and owns all the data.
typedef void* YiniParserHandle;

// Represents a non-owning handle to a section within a parsed document.
// WARNING: This handle is only valid as long as its parent YiniParserHandle exists.
typedef void* YiniSectionHandle;

// Represents a non-owning handle to a value within a section.
// WARNING: This handle is only valid as long as its parent YiniParserHandle exists.
typedef void* YiniValueHandle;


// --- Enums ---

// Value types returned by the API.
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


// --- Parser Lifecycle & Error Handling ---

// Creates a handle by parsing a YINI source string.
// Returns a handle on success, or NULL on failure.
YINI_API YiniParserHandle yini_parser_create(const char* source);

// Creates a handle by parsing a YINI file from disk.
// Returns a handle on success, or NULL on failure.
YINI_API YiniParserHandle yini_parser_create_from_file(const char* filename);

// Destroys a parser handle and frees all associated memory.
YINI_API void yini_parser_destroy(YiniParserHandle parser);

// Gets the last error message from a handle that failed to create.
// The returned string must be freed by calling yini_free_string().
YINI_API const char* yini_parser_get_error(YiniParserHandle parser);


// --- Section Access ---

// Gets the number of sections in the document.
YINI_API int yini_parser_get_section_count(YiniParserHandle parser);

// Gets the names of all sections.
// The returned array and its strings must be freed by calling yini_free_string_array().
YINI_API const char** yini_parser_get_section_names(YiniParserHandle parser, int* count);

// Gets a handle to a specific section by name.
YINI_API YiniSectionHandle yini_parser_get_section(YiniParserHandle parser, const char* name);

// Gets the number of key-value pairs in a section.
YINI_API int yini_section_get_key_count(YiniSectionHandle section);

// Gets the keys of all entries in a section.
// The returned array and its strings must be freed by calling yini_free_string_array().
YINI_API const char** yini_section_get_keys(YiniSectionHandle section, int* count);


// --- Value Access ---

// Gets a handle to a value within a section by its key.
YINI_API YiniValueHandle yini_section_get_value(YiniSectionHandle section, const char* key);

// Gets the type of a value.
YINI_API YiniValueType yini_value_get_type(YiniValueHandle value);

// Gets the integer representation of a value.
YINI_API int64_t yini_value_get_integer(YiniValueHandle value);

// Gets the float representation of a value.
YINI_API double yini_value_get_float(YiniValueHandle value);

// Gets the boolean representation of a value.
YINI_API bool yini_value_get_boolean(YiniValueHandle value);

// Gets the string representation of a value.
// The returned string must be freed by calling yini_free_string().
YINI_API const char* yini_value_get_string(YiniValueHandle value);


// --- Array Access ---

// Gets the number of elements in an array value.
YINI_API int yini_value_get_array_size(YiniValueHandle value);

// Gets a handle to an element within an array value by its index.
YINI_API YiniValueHandle yini_value_get_array_element(YiniValueHandle value, int index);


// --- Memory Management ---

// Frees a single string returned by the API.
YINI_API void yini_free_string(const char* str);

// Frees an array of strings returned by the API.
YINI_API void yini_free_string_array(const char** array, int count);


// --- YMETA Utility Functions ---

// Compiles a YINI file to a binary .ymeta file.
YINI_API bool yini_compile_to_ymeta(const char* input_file, const char* output_file);

// Decompiles a binary .ymeta file to a text-based YINI file.
YINI_API bool yini_decompile_from_ymeta(const char* input_file, const char* output_file);


#ifdef __cplusplus
}
#endif

#endif // YINI_C_API_H