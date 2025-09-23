#ifndef YINI_INTEROP_H
#define YINI_INTEROP_H

#include <stdint.h>
#include <stdbool.h>

// --- Platform-specific DLL export/import macros ---
#if defined(_WIN32)
    #ifdef YINI_EXPORTS
        #define YINI_API __declspec(dllexport)
    #else
        #define YINI_API __declspec(dllimport)
    #endif
#else
    #define YINI_API
#endif

// --- Opaque Handles ---
// In C++, these will be pointers to the corresponding C++ AST nodes.
struct YiniHandle;       // Represents the entire loaded file
struct YiniValueHandle;  // Represents a single YINI value
struct YiniArrayHandle;  // Represents a YINI array
struct YiniObjectHandle; // Represents a YINI object/map
typedef struct YiniHandle YiniHandle;
typedef struct YiniValueHandle YiniValueHandle;
typedef struct YiniArrayHandle YiniArrayHandle;
typedef struct YiniObjectHandle YiniObjectHandle;

// --- Value Type Enum ---
typedef enum
{
    YINI_TYPE_UNINITIALIZED,
    YINI_TYPE_STRING,
    YINI_TYPE_INT64,
    YINI_TYPE_DOUBLE,
    YINI_TYPE_BOOL,
    YINI_TYPE_ARRAY,
    YINI_TYPE_PATH,
    YINI_TYPE_COORD,
    YINI_TYPE_COLOR,
    YINI_TYPE_OBJECT,
} Yini_Value_Type;


#ifdef __cplusplus
extern "C" {
#endif

// --- API Functions ---

/**
 * @brief Loads and processes a YINI file from the given path.
 *
 * This function loads the specified .yini file, processes all includes
 * and inheritance rules, and returns a handle to the resulting data structure.
 *
 * @param filepath The path to the .yini file.
 * @return A handle to the loaded YINI data, or NULL if loading fails.
 *         The returned handle must be freed with yini_free().
 */
YINI_API YiniHandle* yini_load(const char* filepath);

/**
 * @brief Frees the memory associated with a YiniHandle.
 *
 * @param handle The handle returned by yini_load().
 */
YINI_API void yini_free(YiniHandle* handle);

/**
 * @brief Retrieves a string value from the YINI data.
 *
 * @param handle The handle to the YINI data.
 * @param section The name of the section containing the key.
 * @param key The name of the key.
 * @param out_buffer The buffer to write the string into.
 * @param buffer_size The size of the output buffer.
 * @return The number of bytes written to the buffer (excluding null terminator),
 *         or -1 if the key is not found or is not a string.
 */
YINI_API int yini_get_string(YiniHandle* handle, const char* section, const char* key, char* out_buffer, int buffer_size);

/**
 * @brief Retrieves a 64-bit integer value.
 *
 * @param handle The handle to the YINI data.
 * @param section The name of the section.
 * @param key The name of the key.
 * @param out_value A pointer to store the retrieved integer.
 * @return 1 on success, 0 on failure (key not found or type mismatch).
 */
YINI_API int yini_get_int64(YiniHandle* handle, const char* section, const char* key, int64_t* out_value);

/**
 * @brief Retrieves a double-precision float value.
 *
 * @param handle The handle to the YINI data.
 * @param section The name of the section.
 * @param key The name of the key.
 * @param out_value A pointer to store the retrieved double.
 * @return 1 on success, 0 on failure.
 */
YINI_API int yini_get_double(YiniHandle* handle, const char* section, const char* key, double* out_value);

/**
 * @brief Retrieves a boolean value.
 *
 * @param handle The handle to the YINI data.
 * @param section The name of the section.
 * @param key The name of the key.
 * @param out_value A pointer to store the retrieved boolean.
 * @return 1 on success, 0 on failure.
 */
YINI_API int yini_get_bool(YiniHandle* handle, const char* section, const char* key, bool* out_value);

/**
 * @brief Retrieves any value from the YINI data, serialized as a JSON string.
 *
 * This is the most flexible way to retrieve complex types like arrays and objects.
 *
 * @param handle The handle to the YINI data.
 * @param section The name of the section.
 * @param key The name of the key.
 * @param out_buffer The buffer to write the JSON string into.
 * @param buffer_size The size of the output buffer.
 * @return The number of bytes written to the buffer (excluding null terminator),
 *         or -1 if the key is not found.
 */
YINI_API int yini_get_value_as_json(YiniHandle* handle, const char* section, const char* key, char* out_buffer, int buffer_size);


// --- Granular API ---

/**
 * @brief Gets a handle to a specific value in the YINI data.
 * @param handle The main YINI handle.
 * @param section The section of the value.
 * @param key The key of the value.
 * @return A handle to the value, or NULL if not found. This handle is a "view" and should not be freed.
 */
YINI_API YiniValueHandle* yini_get_value(YiniHandle* handle, const char* section, const char* key);

/**
 * @brief Gets the data type of a value handle.
 * @param value_handle A handle to a YINI value.
 * @return The type of the value as a Yini_Value_Type enum.
 */
YINI_API Yini_Value_Type yini_value_get_type(YiniValueHandle* value_handle);

/**
 * @brief Gets the string representation of a value.
 * @param value_handle The handle to a string value.
 * @return The number of bytes written, or -1 on failure/type mismatch.
 */
YINI_API int yini_value_as_string(YiniValueHandle* value_handle, char* out_buffer, int buffer_size);

/**
 * @brief Gets the path string of a value.
 * @param value_handle The handle to a path value.
 * @return The number of bytes written, or -1 on failure/type mismatch.
 */
YINI_API int yini_value_as_path(YiniValueHandle* value_handle, char* out_buffer, int buffer_size);

/**
 * @brief Gets the int64 representation of a value.
 * @param value_handle The handle to an int64 value.
 * @return 1 on success, 0 on failure/type mismatch.
 */
YINI_API int yini_value_as_int64(YiniValueHandle* value_handle, int64_t* out_value);

/**
 * @brief Gets the double representation of a value.
 * @param value_handle The handle to a double or int64 value.
 * @return 1 on success, 0 on failure/type mismatch.
 */
YINI_API int yini_value_as_double(YiniValueHandle* value_handle, double* out_value);

/**
 * @brief Gets the boolean representation of a value.
 * @param value_handle The handle to a boolean value.
 * @return 1 on success, 0 on failure/type mismatch.
 */
YINI_API int yini_value_as_bool(YiniValueHandle* value_handle, bool* out_value);

/**
 * @brief Gets a handle to an array value.
 * @param value_handle The handle to a YINI value.
 * @return A handle to the array, or NULL if the value is not an array.
 */
YINI_API YiniArrayHandle* yini_value_as_array(YiniValueHandle* value_handle);

/**
 * @brief Gets a handle to an object value.
 * @param value_handle The handle to a YINI value.
 * @return A handle to the object, or NULL if the value is not an object.
 */
YINI_API YiniObjectHandle* yini_value_as_object(YiniValueHandle* value_handle);

// Array Access
/**
 * @brief Gets the number of elements in an array.
 * @param array_handle A handle to a YINI array.
 * @return The number of elements, or -1 on error.
 */
YINI_API int yini_array_get_size(YiniArrayHandle* array_handle);

/**
 * @brief Gets a handle to a value at a specific index in an array.
 * @param array_handle A handle to a YINI array.
 * @param index The index of the element.
 * @return A handle to the value at the specified index, or NULL if out of bounds.
 */
YINI_API YiniValueHandle* yini_array_get_value(YiniArrayHandle* array_handle, int index);

/**
 * @brief Serializes a generic value handle to a JSON string.
 * @param value_handle The handle to any YINI value.
 * @param out_buffer The buffer to write the JSON string into.
 * @param buffer_size The size of the output buffer.
 * @return The number of bytes written, or -1 on failure.
 */
YINI_API int yini_value_as_json(YiniValueHandle* value_handle, char* out_buffer, int buffer_size);


#ifdef __cplusplus
}
#endif

#endif // YINI_INTEROP_H
