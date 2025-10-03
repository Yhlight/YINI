/**
 * @file YiniCApi.h
 * @brief Defines the C-style API for interoperability with the YINI library.
 * @ingroup Interop
 *
 * @details This header provides a stable C interface for the YINI C++ library,
 * allowing it to be used from other programming languages like C#, Python, or C.
 * The API is based on opaque handles (`Yini_ManagerHandle`, `Yini_ValueHandle`)
 * to manage the lifecycle of objects.
 *
 * All created handles must be explicitly destroyed using the corresponding
 * `_destroy` function to prevent memory leaks.
 */
#ifndef YINI_C_API_H
#define YINI_C_API_H

#include <stdbool.h>

#ifdef _WIN32
    #define YINI_API __declspec(dllexport)
#else
    #define YINI_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

    /**
     * @defgroup CAPI YINI C API
     * @brief The C-style procedural API for interacting with the YINI library.
     * @ingroup Interop
     */

    /// @name Opaque Handles
    /// @{

    /**
     * @brief An opaque handle to a YiniManager instance.
     * @ingroup CAPI
     */
    typedef void* Yini_ManagerHandle;

    /**
     * @brief An opaque handle to a YiniValue instance.
     * @ingroup CAPI
     */
    typedef void* Yini_ValueHandle;

    /// @}

    /**
     * @brief Enumeration of all possible types a YiniValue can hold.
     * @ingroup CAPI
     */
    typedef enum {
        YINI_TYPE_NULL,   ///< The value is null or uninitialized.
        YINI_TYPE_BOOL,   ///< The value is a boolean (`true` or `false`).
        YINI_TYPE_DOUBLE, ///< The value is a floating-point number.
        YINI_TYPE_STRING, ///< The value is a string.
        YINI_TYPE_ARRAY,  ///< The value is an array of other values.
        YINI_TYPE_MAP,    ///< The value is a map of key-value pairs.
        YINI_TYPE_DYNA    ///< The value is a dynamic, updatable value.
    } Yini_ValueType;

    /// @name Manager Functions
    /// @{

    /**
     * @brief Creates a new YiniManager instance.
     * @ingroup CAPI
     * @return A handle to the new manager. The caller is responsible for destroying it
     *         with `yini_manager_destroy()`.
     */
    YINI_API Yini_ManagerHandle yini_manager_create();

    /**
     * @brief Destroys a YiniManager instance and frees its resources.
     * @ingroup CAPI
     * @param manager The handle to the manager to destroy.
     */
    YINI_API void yini_manager_destroy(Yini_ManagerHandle manager);

    /**
     * @brief Loads and parses a YINI file.
     * @ingroup CAPI
     * @param manager The manager handle.
     * @param filepath The path to the .yini file.
     * @return `true` on success, `false` on failure (e.g., file not found, parsing error).
     */
    YINI_API bool yini_manager_load(Yini_ManagerHandle manager, const char* filepath);

    /**
     * @brief Saves any modified dynamic values back to the source file.
     * @ingroup CAPI
     * @param manager The manager handle.
     */
    YINI_API void yini_manager_save_changes(Yini_ManagerHandle manager);

    /// @}

    /// @name Value Get/Set on Manager
    /// @{

    /**
     * @brief Retrieves a value from the manager.
     * @ingroup CAPI
     * @param manager The manager handle.
     * @param section The section name.
     * @param key The key name.
     * @return A handle to the requested value, or `NULL` if not found. The caller is
     *         responsible for destroying the returned handle with `yini_value_destroy()`.
     */
    YINI_API Yini_ValueHandle yini_manager_get_value(Yini_ManagerHandle manager, const char* section, const char* key);

    /**
     * @brief Sets the value of a dynamic key in the manager.
     * @ingroup CAPI
     * @param manager The manager handle.
     * @param section The section name.
     * @param key The key name.
     * @param value_handle A handle to the value to set. The manager takes ownership of the value.
     */
    YINI_API void yini_manager_set_value(Yini_ManagerHandle manager, const char* section, const char* key, Yini_ValueHandle value_handle);

    /// @}

    /// @name Value Handle Functions
    /// @{

    /**
     * @brief Destroys a YiniValue handle and frees its resources.
     * @ingroup CAPI
     * @param handle The value handle to destroy.
     */
    YINI_API void yini_value_destroy(Yini_ValueHandle handle);

    /**
     * @brief Gets the data type of the value held by the handle.
     * @ingroup CAPI
     * @param handle The value handle.
     * @return The `Yini_ValueType` enumeration for the value.
     */
    YINI_API Yini_ValueType yini_value_get_type(Yini_ValueHandle handle);

    /// @}

    /// @name Create Value Handles
    /// @{

    /**
     * @brief Creates a new YiniValue handle containing a double.
     * @ingroup CAPI
     * @param value The double value.
     * @return A new value handle. The caller is responsible for destroying it.
     */
    YINI_API Yini_ValueHandle yini_value_create_double(double value);

    /**
     * @brief Creates a new YiniValue handle containing a string.
     * @ingroup CAPI
     * @param value The string value.
     * @return A new value handle. The caller is responsible for destroying it.
     */
    YINI_API Yini_ValueHandle yini_value_create_string(const char* value);

    /**
     * @brief Creates a new YiniValue handle containing a boolean.
     * @ingroup CAPI
     * @param value The boolean value.
     * @return A new value handle. The caller is responsible for destroying it.
     */
    YINI_API Yini_ValueHandle yini_value_create_bool(bool value);

    /**
     * @brief Creates a new, empty YiniValue array handle.
     * @ingroup CAPI
     * @return A new array value handle. The caller is responsible for destroying it.
     */
    YINI_API Yini_ValueHandle yini_value_create_array();

    /**
     * @brief Creates a new, empty YiniValue map handle.
     * @ingroup CAPI
     * @return A new map value handle. The caller is responsible for destroying it.
     */
    YINI_API Yini_ValueHandle yini_value_create_map();

    /// @}

    /// @name Get Data from Value Handles
    /// @{

    /**
     * @brief Retrieves a double from a value handle.
     * @ingroup CAPI
     * @param handle The value handle. Must be of type `YINI_TYPE_DOUBLE`.
     * @param[out] out_value Pointer to a double to store the result.
     * @return `true` if the value was successfully retrieved, `false` otherwise.
     */
    YINI_API bool yini_value_get_double(Yini_ValueHandle handle, double* out_value);

    /**
     * @brief Retrieves a string from a value handle.
     * @ingroup CAPI
     * @details This function follows a two-call pattern. First, call it with `out_buffer`
     * as `NULL` to get the required buffer size. Then, allocate the buffer and
     * call it again to get the string.
     * @param handle The value handle. Must be of type `YINI_TYPE_STRING`.
     * @param[out] out_buffer A buffer to copy the string into, or `NULL`.
     * @param buffer_size The size of `out_buffer`.
     * @return The required buffer size (including null terminator), or -1 on error.
     */
    YINI_API int yini_value_get_string(Yini_ValueHandle handle, char* out_buffer, int buffer_size);

    /**
     * @brief Retrieves a boolean from a value handle.
     * @ingroup CAPI
     * @param handle The value handle. Must be of type `YINI_TYPE_BOOL`.
     * @param[out] out_value Pointer to a bool to store the result.
     * @return `true` if the value was successfully retrieved, `false` otherwise.
     */
    YINI_API bool yini_value_get_bool(Yini_ValueHandle handle, bool* out_value);

    /**
     * @brief Retrieves the underlying value from a dynamic value handle.
     * @ingroup CAPI
     * @param handle The value handle. Must be of type `YINI_TYPE_DYNA`.
     * @return A new handle to the underlying value. The caller is responsible
     *         for destroying it. Returns `NULL` on error.
     */
    YINI_API Yini_ValueHandle yini_value_get_dyna_value(Yini_ValueHandle handle);

    /// @}

    /// @name Array Manipulation
    /// @{

    /**
     * @brief Gets the number of elements in an array.
     * @ingroup CAPI
     * @param handle The array value handle. Must be of type `YINI_TYPE_ARRAY`.
     * @return The size of the array, or -1 on error.
     */
    YINI_API int yini_array_get_size(Yini_ValueHandle handle);

    /**
     * @brief Gets an element from an array at a specific index.
     * @ingroup CAPI
     * @param handle The array value handle.
     * @param index The index of the element.
     * @return A new handle to the element. The caller is responsible for destroying it.
     *         Returns `NULL` if the index is out of bounds or on error.
     */
    YINI_API Yini_ValueHandle yini_array_get_element(Yini_ValueHandle handle, int index);

    /**
     * @brief Adds an element to the end of an array.
     * @ingroup CAPI
     * @param array_handle The array value handle.
     * @param element_handle The element handle to add. The array takes ownership of the element.
     */
    YINI_API void yini_array_add_element(Yini_ValueHandle array_handle, Yini_ValueHandle element_handle);

    /// @}

    /// @name Map Manipulation
    /// @{

    /**
     * @brief Gets the number of key-value pairs in a map.
     * @ingroup CAPI
     * @param handle The map value handle. Must be of type `YINI_TYPE_MAP`.
     * @return The size of the map, or -1 on error.
     */
    YINI_API int yini_map_get_size(Yini_ValueHandle handle);

    /**
     * @brief Gets the value from a map at a specific index.
     * @ingroup CAPI
     * @param handle The map value handle.
     * @param index The index of the key-value pair.
     * @return A new handle to the value. The caller is responsible for destroying it.
     *         Returns `NULL` if the index is out of bounds or on error.
     */
    YINI_API Yini_ValueHandle yini_map_get_value_at(Yini_ValueHandle handle, int index);

    /**
     * @brief Gets the key from a map at a specific index.
     * @ingroup CAPI
     * @param handle The map value handle.
     * @param index The index of the key-value pair.
     * @return A pointer to the key string. The lifetime of this pointer is tied to
     *         the map handle. Do not free it. Returns `NULL` on error.
     */
    YINI_API const char* yini_map_get_key_at(Yini_ValueHandle handle, int index);

    /**
     * @brief Sets a key-value pair in a map.
     * @ingroup CAPI
     * @param map_handle The map value handle.
     * @param key The key for the new pair.
     * @param value_handle The value handle for the new pair. The map takes ownership of the value.
     */
    YINI_API void yini_map_set_value(Yini_ValueHandle map_handle, const char* key, Yini_ValueHandle value_handle);

    /// @}

#ifdef __cplusplus
}
#endif

#endif // YINI_C_API_H