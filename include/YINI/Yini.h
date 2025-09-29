/**
 * @file Yini.h
 * @brief The C-style public API for the YINI library.
 *
 * This file defines the public interface for parsing, manipulating, and querying
 * YINI documents. It uses opaque handles to manage the underlying C++ objects.
 */

#ifndef YINI_C_API_H
#define YINI_C_API_H

#ifdef _WIN32
    #ifdef YINI_EXPORTS
        #define YINI_API __declspec(dllexport)
    #else
        #define YINI_API __declspec(dllimport)
    #endif
#else
    #define YINI_API
#endif

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief An opaque handle representing a loaded YINI document.
 */
typedef struct YiniDocumentHandle YiniDocumentHandle;

/**
 * @brief An opaque handle representing a section within a YINI document.
 */
typedef struct YiniSectionHandle YiniSectionHandle;

/**
 * @brief An opaque handle representing a value within a YINI document.
 */
typedef struct YiniValueHandle YiniValueHandle;

/**
 * @brief Enumerates the possible types a YiniValue can hold.
 */
typedef enum {
    YINI_TYPE_NONE,      /**< Represents a null or uninitialized value. */
    YINI_TYPE_STRING,    /**< A string value. */
    YINI_TYPE_INT,       /**< An integer value. */
    YINI_TYPE_DOUBLE,    /**< A double-precision floating-point value. */
    YINI_TYPE_BOOL,      /**< A boolean value (true or false). */
    YINI_TYPE_ARRAY,     /**< An array of values. */
    YINI_TYPE_LIST,      /**< A linked-list-style collection of values. */
    YINI_TYPE_SET,       /**< A collection of unique values. */
    YINI_TYPE_MAP,       /**< A key-value map. */
    YINI_TYPE_DYNA,      /**< A dynamic value that can be written back to the source file. */
    YINI_TYPE_COORD,     /**< A 2D or 3D coordinate value. */
    YINI_TYPE_COLOR,     /**< A color value. */
    YINI_TYPE_PATH       /**< A file path value. */
} YiniType;

//==============================================================================
// Document API
//==============================================================================

/**
 * @brief Parses a string containing YINI data into a document handle.
 * @param content The null-terminated string containing the YINI data to parse.
 * @param error_buffer A buffer to write any parsing error messages to. Can be NULL.
 * @param buffer_size The size of the error_buffer.
 * @return A handle to the parsed document, or NULL if parsing fails. The caller is responsible for freeing this handle with yini_free_document().
 */
YINI_API YiniDocumentHandle* yini_parse(const char* content, char* error_buffer, int buffer_size);

/**
 * @brief Frees all memory associated with a YINI document handle.
 * @param handle The document handle to free.
 */
YINI_API void yini_free_document(YiniDocumentHandle* handle);

/**
 * @brief Gets the number of sections in the document.
 * @param handle The document handle.
 * @return The number of sections.
 */
YINI_API int yini_get_section_count(const YiniDocumentHandle* handle);

/**
 * @brief Gets a section by its index.
 * @param handle The document handle.
 * @param index The zero-based index of the section.
 * @return A handle to the section, or NULL if the index is out of bounds.
 */
YINI_API const YiniSectionHandle* yini_get_section_by_index(const YiniDocumentHandle* handle, int index);

/**
 * @brief Gets a section by its name.
 * @param handle The document handle.
 * @param name The name of the section to find.
 * @return A handle to the section, or NULL if not found.
 */
YINI_API const YiniSectionHandle* yini_get_section_by_name(const YiniDocumentHandle* handle, const char* name);

/**
 * @brief Sets a string value for a given key in a section. Creates the section or key if it doesn't exist.
 * @param handle The document handle.
 * @param section The name of the section.
 * @param key The name of the key.
 * @param value The string value to set.
 */
YINI_API void yini_set_string_value(YiniDocumentHandle* handle, const char* section, const char* key, const char* value);

/**
 * @brief Sets an integer value for a given key in a section.
 * @param handle The document handle.
 * @param section The name of the section.
 * @param key The name of the key.
 * @param value The integer value to set.
 */
YINI_API void yini_set_int_value(YiniDocumentHandle* handle, const char* section, const char* key, int value);

/**
 * @brief Sets a double value for a given key in a section.
 * @param handle The document handle.
 * @param section The name of the section.
 * @param key The name of the key.
 * @param value The double value to set.
 */
YINI_API void yini_set_double_value(YiniDocumentHandle* handle, const char* section, const char* key, double value);

/**
 * @brief Sets a boolean value for a given key in a section.
 * @param handle The document handle.
 * @param section The name of the section.
 * @param key The name of the key.
 * @param value The boolean value to set.
 */
YINI_API void yini_set_bool_value(YiniDocumentHandle* handle, const char* section, const char* key, bool value);

/**
 * @brief Gets the number of macros ([#define]s) in the document.
 * @param handle The document handle.
 * @return The number of defined macros.
 */
YINI_API int yini_get_define_count(const YiniDocumentHandle* handle);

/**
 * @brief Gets a macro definition by its index.
 * @param handle The document handle.
 * @param index The zero-based index of the macro.
 * @param key_buffer A buffer to write the macro's key into.
 * @param key_buffer_size The size of the key_buffer.
 * @return A handle to the macro's value, or NULL if the index is out of bounds.
 */
YINI_API const YiniValueHandle* yini_get_define_by_index(const YiniDocumentHandle* handle, int index, char* key_buffer, int key_buffer_size);

/**
 * @brief Gets a macro definition by its key.
 * @param handle The document handle.
 * @param key The key of the macro to find.
 * @return A handle to the macro's value, or NULL if not found.
 */
YINI_API const YiniValueHandle* yini_get_define_by_key(const YiniDocumentHandle* handle, const char* key);


//==============================================================================
// Section API
//==============================================================================

/**
 * @brief Gets the name of a section.
 * @param section_handle The section handle.
 * @param buffer A buffer to write the name into.
 * @param buffer_size The size of the buffer.
 * @return The required buffer size for the name.
 */
YINI_API int yini_section_get_name(const YiniSectionHandle* section_handle, char* buffer, int buffer_size);

/**
 * @brief Gets the number of key-value pairs in a section.
 * @param section_handle The section handle.
 * @return The number of pairs.
 */
YINI_API int yini_section_get_pair_count(const YiniSectionHandle* section_handle);

/**
 * @brief Gets the key of a key-value pair by its index in the section.
 * @param section_handle The section handle.
 * @param index The zero-based index of the pair.
 * @param buffer A buffer to write the key into.
 * @param buffer_size The size of the buffer.
 * @return The required buffer size for the key.
 */
YINI_API int yini_section_get_pair_key_by_index(const YiniSectionHandle* section_handle, int index, char* buffer, int buffer_size);

/**
 * @brief Gets the value of a key-value pair by its key in the section.
 * @param section_handle The section handle.
 * @param key The key of the value to find.
 * @return A handle to the value, or NULL if not found.
 */
YINI_API const YiniValueHandle* yini_section_get_value_by_key(const YiniSectionHandle* section_handle, const char* key);

/**
 * @brief Gets the number of quick-registration (+=) values in a section.
 * @param section_handle The section handle.
 * @return The number of registered values.
 */
YINI_API int yini_section_get_registration_count(const YiniSectionHandle* section_handle);

/**
 * @brief Gets a quick-registration value by its index.
 * @param section_handle The section handle.
 * @param index The zero-based index of the registered value.
 * @return A handle to the value, or NULL if the index is out of bounds.
 */
YINI_API const YiniValueHandle* yini_section_get_registered_value_by_index(const YiniSectionHandle* section_handle, int index);


//==============================================================================
// Value API
//==============================================================================

/**
 * @brief Gets the type of a value.
 * @param value_handle The value handle.
 * @return The YiniType of the value.
 */
YINI_API YiniType yini_value_get_type(const YiniValueHandle* value_handle);

/**
 * @brief Gets the string content of a value.
 * @param value_handle The value handle.
 * @param buffer A buffer to write the string into.
 * @param buffer_size The size of the buffer.
 * @return The required buffer size for the string. Returns 0 if the value is not a string.
 */
YINI_API int yini_value_get_string(const YiniValueHandle* value_handle, char* buffer, int buffer_size);

/**
 * @brief Gets the integer content of a value.
 * @param value_handle The value handle.
 * @param out_value A pointer to an integer where the value will be stored.
 * @return `true` if the value is an integer and was retrieved successfully, `false` otherwise.
 */
YINI_API bool yini_value_get_int(const YiniValueHandle* value_handle, int* out_value);

/**
 * @brief Gets the double content of a value.
 * @param value_handle The value handle.
 * @param out_value A pointer to a double where the value will be stored.
 * @return `true` if the value is a double and was retrieved successfully, `false` otherwise.
 */
YINI_API bool yini_value_get_double(const YiniValueHandle* value_handle, double* out_value);

/**
 * @brief Gets the boolean content of a value.
 * @param value_handle The value handle.
 * @param out_value A pointer to a boolean where the value will be stored.
 * @return `true` if the value is a boolean and was retrieved successfully, `false` otherwise.
 */
YINI_API bool yini_value_get_bool(const YiniValueHandle* value_handle, bool* out_value);

/**
 * @brief Gets the coordinate content of a value.
 * @param value_handle The value handle.
 * @param x Pointer to store the x-component.
 * @param y Pointer to store the y-component.
 * @param z Pointer to store the z-component.
 * @param is_3d Pointer to store whether the coordinate is 3D.
 * @return `true` if the value is a coordinate and was retrieved successfully, `false` otherwise.
 */
YINI_API bool yini_value_get_coord(const YiniValueHandle* value_handle, double* x, double* y, double* z, bool* is_3d);

/**
 * @brief Gets the color content of a value.
 * @param value_handle The value handle.
 * @param r Pointer to store the red component.
 * @param g Pointer to store the green component.
 * @param b Pointer to store the blue component.
 * @return `true` if the value is a color and was retrieved successfully, `false` otherwise.
 */
YINI_API bool yini_value_get_color(const YiniValueHandle* value_handle, unsigned char* r, unsigned char* g, unsigned char* b);

/**
 * @brief Gets the path content of a value.
 * @param value_handle The value handle.
 * @param buffer A buffer to write the path into.
 * @param buffer_size The size of the buffer.
 * @return The required buffer size for the path. Returns 0 if the value is not a path.
 */
YINI_API int yini_value_get_path(const YiniValueHandle* value_handle, char* buffer, int buffer_size);

/**
 * @brief Gets the size of an array value.
 * @param value_handle The value handle.
 * @return The number of elements in the array, or 0 if the value is not an array.
 */
YINI_API int yini_array_get_size(const YiniValueHandle* value_handle);

/**
 * @brief Gets an element from an array value by its index.
 * @param value_handle The array value handle.
 * @param index The zero-based index of the element.
 * @return A handle to the element's value, or NULL if the index is out of bounds or the handle is not an array.
 */
YINI_API const YiniValueHandle* yini_array_get_value_by_index(const YiniValueHandle* value_handle, int index);

//==============================================================================
// List API
//==============================================================================

/**
 * @brief Gets the size of a list value.
 * @param value_handle The value handle.
 * @return The number of elements in the list, or 0 if the value is not a list.
 */
YINI_API int yini_list_get_size(const YiniValueHandle* value_handle);

/**
 * @brief Gets an element from a list value by its index.
 * @param value_handle The list value handle.
 * @param index The zero-based index of the element.
 * @return A handle to the element's value, or NULL if the index is out of bounds or the handle is not a list.
 */
YINI_API const YiniValueHandle* yini_list_get_value_by_index(const YiniValueHandle* value_handle, int index);

//==============================================================================
// Set API
//==============================================================================

/**
 * @brief Gets the size of a set value.
 * @param value_handle The value handle.
 * @return The number of elements in the set, or 0 if the value is not a set.
 */
YINI_API int yini_set_get_size(const YiniValueHandle* value_handle);

/**
 * @brief Gets an element from a set value by its index.
 * @param value_handle The set value handle.
 * @param index The zero-based index of the element.
 * @return A handle to the element's value, or NULL if the index is out of bounds or the handle is not a set.
 */
YINI_API const YiniValueHandle* yini_set_get_value_by_index(const YiniValueHandle* value_handle, int index);

//==============================================================================
// Map API
//==============================================================================

/**
 * @brief Gets the size of a map value.
 * @param value_handle The value handle.
 * @return The number of key-value pairs in the map, or 0 if the value is not a map.
 */
YINI_API int yini_map_get_size(const YiniValueHandle* value_handle);

/**
 * @brief Gets a key from a map value by its index.
 * @param value_handle The map value handle.
 * @param index The zero-based index of the key.
 * @param buffer A buffer to write the key into.
 * @param buffer_size The size of the buffer.
 * @return The required buffer size for the key.
 */
YINI_API int yini_map_get_key_by_index(const YiniValueHandle* value_handle, int index, char* buffer, int buffer_size);

/**
 * @brief Gets a value from a map by its key.
 * @param value_handle The map value handle.
 * @param key The key of the value to find.
 * @return A handle to the value, or NULL if not found.
 */
YINI_API const YiniValueHandle* yini_map_get_value_by_key(const YiniValueHandle* value_handle, const char* key);

#ifdef __cplusplus
}
#endif

#endif // YINI_C_API_H