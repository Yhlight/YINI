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
 * @brief Sets the value for a given section and key.
 * @param manager The manager handle.
 * @param section The name of the section.
 * @param key The name of the key.
 * @param value_handle The handle to the value to set. The value is copied.
 */
YINI_API void yini_manager_set_value(Yini_ManagerHandle manager, const char* section, const char* key, Yini_ValueHandle value_handle);

//================================================================================
// Schema and Validation Functions
//================================================================================

/**
 * @brief Validates the loaded configuration against its schema.
 * @param manager The manager handle.
 * @return True if validation succeeds or if there is no schema, false otherwise.
 */
YINI_API bool yini_manager_validate(Yini_ManagerHandle manager);

/**
 * @brief Gets the number of validation errors found during the last validation.
 * @param manager The manager handle.
 * @return The number of validation errors.
 */
YINI_API int yini_manager_get_validation_error_count(Yini_ManagerHandle manager);

/**
 * @brief Gets a specific validation error message.
 *
 * This function should be called twice. First, with `out_buffer` as NULL to get the
 * required buffer size. Then, after allocating a buffer of that size, call it
 * again to get the error message.
 *
 * @param manager The manager handle.
 * @param index The index of the error.
 * @param out_buffer The buffer to write the message into, or NULL.
 * @param buffer_size The size of the buffer.
 * @return If `out_buffer` is NULL, returns the required buffer size. Otherwise,
 *         returns the number of characters written. Returns a negative value on error.
 */
YINI_API int yini_manager_get_validation_error(Yini_ManagerHandle manager, int index, char* out_buffer, int buffer_size);

//================================================================================
// Metadata Functions
//================================================================================

/**
 * @brief Gets the documentation comment for a specific section.
 * @param manager The manager handle.
 * @param section_name The name of the section.
 * @param out_buffer The buffer to write the comment into.
 * @param buffer_size The size of the buffer.
 * @return The number of characters written, or required buffer size.
 */
YINI_API int yini_manager_get_section_doc_comment(Yini_ManagerHandle manager, const char* section_name, char* out_buffer, int buffer_size);

/**
 * @brief Gets the documentation comment for a specific key.
 * @param manager The manager handle.
 * @param section_name The name of the section.
 * @param key_name The name of the key.
 * @param out_buffer The buffer to write the comment into.
 * @param buffer_size The size of the buffer.
 * @return The number of characters written, or required buffer size.
 */
YINI_API int yini_manager_get_key_doc_comment(Yini_ManagerHandle manager, const char* section_name, const char* key_name, char* out_buffer, int buffer_size);

/**
 * @brief Gets the inline comment for a specific key.
 * @param manager The manager handle.
 * @param section_name The name of the section.
 * @param key_name The name of the key.
 * @param out_buffer The buffer to write the comment into.
 * @param buffer_size The size of the buffer.
 * @return The number of characters written, or required buffer size.
 */
YINI_API int yini_manager_get_key_inline_comment(Yini_ManagerHandle manager, const char* section_name, const char* key_name, char* out_buffer, int buffer_size);

/**
 * @brief Sets the documentation comment for a specific section.
 * @param manager The manager handle.
 * @param section_name The name of the section.
 * @param comment The new comment string.
 */
YINI_API void yini_manager_set_section_doc_comment(Yini_ManagerHandle manager, const char* section_name, const char* comment);

/**
 * @brief Sets the documentation comment for a specific key.
 * @param manager The manager handle.
 * @param section_name The name of the section.
 * @param key_name The name of the key.
 * @param comment The new comment string.
 */
YINI_API void yini_manager_set_key_doc_comment(Yini_ManagerHandle manager, const char* section_name, const char* key_name, const char* comment);

/**
 * @brief Sets the inline comment for a specific key.
 * @param manager The manager handle.
 * @param section_name The name of the section.
 * @param key_name The name of the key.
 * @param comment The new comment string.
 */
YINI_API void yini_manager_set_key_inline_comment(Yini_ManagerHandle manager, const char* section_name, const char* key_name, const char* comment);

//================================================================================
// Iteration Functions
//================================================================================

/**
 * @brief Gets the number of resolved sections.
 * @param manager The manager handle.
 * @return The number of sections.
 */
YINI_API int yini_manager_get_section_count(Yini_ManagerHandle manager);

/**
 * @brief Gets the name of a section at a specific index.
 * @param manager The manager handle.
 * @param index The index of the section.
 * @param out_buffer The buffer to write the name into.
 * @param buffer_size The size of the buffer.
 * @return The number of characters written, or required buffer size.
 */
YINI_API int yini_manager_get_section_name_at(Yini_ManagerHandle manager, int index, char* out_buffer, int buffer_size);

/**
 * @brief Gets the number of keys in a specific section.
 * @param manager The manager handle.
 * @param section_name The name of the section.
 * @return The number of keys in the section, or -1 if the section doesn't exist.
 */
YINI_API int yini_manager_get_key_count_in_section(Yini_ManagerHandle manager, const char* section_name);

/**
 * @brief Gets the name of a key in a section at a specific index.
 * @param manager The manager handle.
 * @param section_name The name of the section.
 * @param index The index of the key.
 * @param out_buffer The buffer to write the name into.
 * @param buffer_size The size of the buffer.
 * @return The number of characters written, or required buffer size.
 */
YINI_API int yini_manager_get_key_name_at(Yini_ManagerHandle manager, const char* section_name, int index, char* out_buffer, int buffer_size);


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