/**
 * @file YiniCApi.h
 * @brief Defines the C-style API for interacting with the YINI library.
 *
 * This API is designed for interoperability with other languages that can call
 * into C libraries, such as C#, Python, and Rust. It uses opaque pointers
 * (handles) to manage the lifetime of YINI objects.
 */
#pragma once

#include <stdbool.h>
#include <stddef.h>

#if defined(_WIN32)
#ifdef YINI_BUILD_DLL
#define YINI_API __declspec(dllexport)
#else
#define YINI_API __declspec(dllimport)
#endif
#else
#define YINI_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque handle representing a YiniManager instance.
 */
typedef struct Yini_Manager* Yini_ManagerHandle;

/**
 * @brief Opaque handle representing a YiniValue instance.
 */
typedef struct Yini_Value* Yini_ValueHandle;

/**
 * @enum YiniValueType
 * @brief Enumerates the possible types of a YiniValue.
 */
typedef enum {
    YiniValueType_Null,
    YiniValueType_Bool,
    YiniValueType_Double,
    YiniValueType_String,
    YiniValueType_Array,
    YiniValueType_Map,
    YiniValueType_Dyna
} YiniValueType;

//================================================================================
// Manager Functions
//================================================================================

/**
 * @brief Creates a new YiniManager instance.
 * @return A handle to the new manager. The caller takes ownership and must free
 *         it with yini_manager_destroy().
 */
YINI_API Yini_ManagerHandle yini_manager_create();

/**
 * @brief Destroys a YiniManager instance and frees its resources.
 * @param manager The manager handle to destroy.
 */
YINI_API void yini_manager_destroy(Yini_ManagerHandle manager);

/**
 * @brief Loads and parses a YINI file.
 * @param manager The manager handle.
 * @param filepath The path to the .yini file.
 * @return True on success, false on failure (e.g., file not found).
 */
YINI_API bool yini_manager_load(Yini_ManagerHandle manager, const char* filepath);

/**
 * @brief Loads and parses a YINI configuration from a string.
 * @param manager The manager handle.
 * @param content The string content to parse.
 * @param virtual_filepath A virtual path to use for error reporting.
 * @return True on success, false on failure.
 */
YINI_API bool yini_manager_load_from_string(Yini_ManagerHandle manager, const char* content,
                                            const char* virtual_filepath);

/**
 * @brief Saves any modified dynamic values back to the original file.
 * @param manager The manager handle.
 */
YINI_API void yini_manager_save_changes(Yini_ManagerHandle manager);

/**
 * @brief Retrieves a value from a specific section and key.
 * @param manager The manager handle.
 * @param section The name of the section.
 * @param key The name of the key.
 * @return A new handle to the value, or NULL if not found. The caller takes
 *         ownership and must free the returned handle with yini_value_destroy().
 */
YINI_API Yini_ValueHandle yini_manager_get_value(Yini_ManagerHandle manager, const char* section, const char* key);

/**
 * @brief Checks if a key exists in a given section.
 * @param manager The manager handle.
 * @param section The name of the section.
 * @param key The name of the key.
 * @return True if the key exists, false otherwise.
 */
YINI_API bool yini_manager_has_key(Yini_ManagerHandle manager, const char* section, const char* key);

/**
 * @brief Gets the last error message that occurred on a manager.
 *
 * This function should be called after an API function returns an error status
 * (e.g., false or NULL). The error string is cleared after this call.
 *
 * @param manager The manager handle.
 * @param out_buffer The buffer to write the error message into, or NULL.
 * @param buffer_size The size of the buffer.
 * @return If `out_buffer` is NULL, returns the required buffer size (including the null terminator).
 *         If `out_buffer` is not NULL, returns the number of characters written to the buffer
 *         (excluding the null terminator). Returns 0 if there is no error.
 */
YINI_API int yini_manager_get_last_error(Yini_ManagerHandle manager, char* out_buffer, int buffer_size);

/**
 * @brief Gets the number of defined macros.
 * @param manager The manager handle.
 * @return The number of macros.
 */
YINI_API int yini_manager_get_macro_count(Yini_ManagerHandle manager);

/**
 * @brief Gets the name of a macro at a specific index.
 *
 * This function should be called twice. First, with `out_buffer` as NULL to get the
 * required buffer size. Then, after allocating a buffer of that size, call it
 * again to get the string.
 *
 * @param manager The manager handle.
 * @param index The index of the macro.
 * @param out_buffer The buffer to write the name into, or NULL.
 * @param buffer_size The size of the buffer.
 * @return If `out_buffer` is NULL, returns the required buffer size (including the null terminator).
 *         If `out_buffer` is not NULL, returns the number of characters written to the buffer
 *         (excluding the null terminator). Returns a negative value on error.
 */
YINI_API int yini_manager_get_macro_name_at(Yini_ManagerHandle manager, int index, char* out_buffer, int buffer_size);

/**
 * @brief Sets the value for a given section and key.
 * @param manager The manager handle.
 * @param section The name of the section.
 * @param key The name of the key.
 * @param value_handle The handle to the value to set. The value is copied.
 */
YINI_API void yini_manager_set_value(Yini_ManagerHandle manager, const char* section, const char* key,
                                     Yini_ValueHandle value_handle);

/**
 * @brief Finds the section and key located at a specific line and column.
 *
 * This function is designed for IDE tooling to identify the symbol under the cursor.
 * It uses a two-call pattern: first call with null buffers to get the required sizes,
 * second call with allocated buffers to get the data.
 *
 * @param manager A handle to the YiniManager instance.
 * @param line The line number to look at (1-based).
 * @param column The column number to look at (1-based).
 * @param out_section Buffer to store the found section name. Can be null.
 * @param section_size The size of the section buffer. On the first call, this returns the required size.
 * @param out_key Buffer to store the found key name. Can be null.
 * @param key_size The size of the key buffer. On the first call, this returns the required size.
 * @return 1 if a key is found at the position, 0 otherwise.
 */
YINI_API int yini_manager_find_key_at_pos(Yini_ManagerHandle manager, int line, int column, char* out_section,
                                          int* section_size, char* out_key, int* key_size);

/**
 * @brief Gets the definition location of a symbol (a key or a macro).
 *
 * For a key, both `section_name` and `symbol_name` must be provided.
 * For a macro, `section_name` should be NULL, and `symbol_name` is the macro name.
 *
 * @param manager A handle to the YiniManager instance.
 * @param section_name The name of the section containing the key, or NULL for a macro.
 * @param symbol_name The name of the key or macro.
 * @param out_filepath Buffer to store the definition file path. Can be null for size query.
 * @param filepath_size The size of the filepath buffer. On the first call, this returns the required size.
 * @param out_line Pointer to store the line number.
 * @param out_column Pointer to store the column number.
 * @return True if the definition is found, false otherwise.
 */
YINI_API bool yini_manager_get_definition_location(Yini_ManagerHandle manager, const char* section_name,
                                                   const char* symbol_name, char* out_filepath, int* filepath_size,
                                                   int* out_line, int* out_column);

//================================================================================
// Value Functions
//================================================================================

/**
 * @brief Destroys a YiniValue instance.
 * @param handle The value handle to destroy.
 */
YINI_API void yini_value_destroy(Yini_ValueHandle handle);

/**
 * @brief Gets the type of a YiniValue.
 * @param handle The value handle.
 * @return The type of the value.
 */
YINI_API YiniValueType yini_value_get_type(Yini_ValueHandle handle);

/**
 * @brief Creates a new YiniValue of type Double.
 * @param value The double value.
 * @return A new handle. The caller takes ownership and must free it with yini_value_destroy().
 */
YINI_API Yini_ValueHandle yini_value_create_double(double value);

/**
 * @brief Creates a new YiniValue of type String.
 * @param value The string value.
 * @return A new handle. The caller takes ownership and must free it with yini_value_destroy().
 */
YINI_API Yini_ValueHandle yini_value_create_string(const char* value);

/**
 * @brief Creates a new YiniValue of type Bool.
 * @param value The boolean value.
 * @return A new handle. The caller takes ownership and must free it with yini_value_destroy().
 */
YINI_API Yini_ValueHandle yini_value_create_bool(bool value);

/**
 * @brief Creates a new, empty YiniValue of type Array.
 * @return A new handle. The caller takes ownership and must free it with yini_value_destroy().
 */
YINI_API Yini_ValueHandle yini_value_create_array();

/**
 * @brief Creates a new, empty YiniValue of type Map.
 * @return A new handle. The caller takes ownership and must free it with yini_value_destroy().
 */
YINI_API Yini_ValueHandle yini_value_create_map();

/**
 * @brief Gets the double value from a YiniValue.
 * @param handle The value handle.
 * @param out_value Pointer to store the result.
 * @return True if the value is a double, false otherwise.
 */
YINI_API bool yini_value_get_double(Yini_ValueHandle handle, double* out_value);

/**
 * @brief Gets the string value from a YiniValue.
 *
 * This function should be called twice. First, with `out_buffer` as NULL to get the
 * required buffer size. Then, after allocating a buffer of that size, call it
 * again to get the string.
 *
 * @param handle The value handle.
 * @param out_buffer The buffer to write the string into, or NULL.
 * @param buffer_size The size of the buffer.
 * @return If `out_buffer` is NULL, returns the required buffer size (including the null terminator).
 *         If `out_buffer` is not NULL, returns the number of characters written to the buffer
 *         (excluding the null terminator). Returns a negative value on error.
 */
YINI_API int yini_value_get_string(Yini_ValueHandle handle, char* out_buffer, int buffer_size);

/**
 * @brief Gets the boolean value from a YiniValue.
 * @param handle The value handle.
 * @param out_value Pointer to store the result.
 * @return True if the value is a boolean, false otherwise.
 */
YINI_API bool yini_value_get_bool(Yini_ValueHandle handle, bool* out_value);

/**
 * @brief Gets the underlying value of a dynamic value.
 * @param handle The handle to a Dyna value.
 * @return A new handle to the inner value, or NULL if the input is not a Dyna value.
 *         The caller takes ownership and must free it with yini_value_destroy().
 */
YINI_API Yini_ValueHandle yini_value_get_dyna_value(Yini_ValueHandle handle);

//================================================================================
// Array Functions
//================================================================================

/**
 * @brief Gets the number of elements in an array value.
 * @param handle The handle to an array value.
 * @return The number of elements, or -1 if the handle is not an array.
 */
YINI_API int yini_array_get_size(Yini_ValueHandle handle);

/**
 * @brief Gets an element from an array value by its index.
 * @param handle The handle to an array value.
 * @param index The index of the element.
 * @return A new handle to the element, or NULL if out of bounds or not an array.
 *         The caller takes ownership and must free it with yini_value_destroy().
 */
YINI_API Yini_ValueHandle yini_array_get_element(Yini_ValueHandle handle, int index);

/**
 * @brief Adds an element to the end of an array value.
 * @param array_handle The handle to an array value.
 * @param element_handle The handle to the element to add. The value is copied.
 *                       The caller retains ownership of element_handle.
 */
YINI_API void yini_array_add_element(Yini_ValueHandle array_handle, Yini_ValueHandle element_handle);

//================================================================================
// Map Functions
//================================================================================

/**
 * @brief Gets the number of key-value pairs in a map value.
 * @param handle The handle to a map value.
 * @return The number of pairs, or -1 if the handle is not a map.
 */
YINI_API int yini_map_get_size(Yini_ValueHandle handle);

/**
 * @brief Gets the value from a map at a specific index.
 * @param handle The handle to a map value.
 * @param index The index of the pair.
 * @return A new handle to the value, or NULL if out of bounds. The caller takes
 *         ownership and must free it with yini_value_destroy().
 */
YINI_API Yini_ValueHandle yini_map_get_value_at(Yini_ValueHandle handle, int index);

/**
 * @brief Gets the key from a map at a specific index.
 *
 * This function should be called twice. First, with `out_buffer` as NULL to get the
 * required buffer size. Then, after allocating a buffer of that size, call it
 * again to get the string.
 *
 * @param handle The handle to a map value.
 * @param index The index of the pair.
 * @param out_buffer The buffer to write the key into, or NULL.
 * @param buffer_size The size of the buffer.
 * @return If `out_buffer` is NULL, returns the required buffer size (including the null terminator).
 *         If `out_buffer` is not NULL, returns the number of characters written to the buffer
 *         (excluding the null terminator). Returns a negative value on error.
 */
YINI_API int yini_map_get_key_at(Yini_ValueHandle handle, int index, char* out_buffer, int buffer_size);

/**
 * @brief Sets a value in a map for a given key.
 * @param map_handle The handle to a map value.
 * @param key The key for the value.
 * @param value_handle The handle to the value to set. The value is copied.
 *                     The caller retains ownership of value_handle.
 */
YINI_API void yini_map_set_value(Yini_ValueHandle map_handle, const char* key, Yini_ValueHandle value_handle);

#ifdef __cplusplus
}
#endif