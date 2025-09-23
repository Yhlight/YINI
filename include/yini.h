#ifndef YINI_C_API_H
#define YINI_C_API_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Opaque pointer to the YINI runtime object.
// The user of this API does not need to know the internal structure.
typedef void* YINI_HANDLE;

// --- Lifecycle Functions ---

/**
 * @brief Loads a YINI configuration from a string.
 * @param content A null-terminated UTF-8 string containing the YINI configuration.
 * @return A handle to the YINI runtime, or NULL if parsing fails.
 */
YINI_HANDLE yini_load_from_string(const char* content);

/**
 * @brief Frees all resources associated with a YINI handle.
 * @param handle The handle returned by yini_load_from_string.
 */
void yini_free(YINI_HANDLE handle);


// --- Value Retrieval Functions ---

/**
 * @brief Retrieves an integer value.
 * @param handle The YINI handle.
 * @param section The name of the section.
 * @param key The name of the key.
 * @param out_value Pointer to a long long to store the result.
 * @return true if the key was found and is an integer, false otherwise.
 */
bool yini_get_integer(YINI_HANDLE handle, const char* section, const char* key, long long* out_value);

/**
 * @brief Retrieves a floating-point value.
 * @param handle The YINI handle.
 * @param section The name of the section.
 * @param key The name of the key.
 * @param out_value Pointer to a double to store the result.
 * @return true if the key was found and is a float or integer, false otherwise.
 */
bool yini_get_float(YINI_HANDLE handle, const char* section, const char* key, double* out_value);

/**
 * @brief Retrieves a boolean value.
 * @param handle The YINI handle.
 * @param section The name of the section.
 * @param key The name of the key.
 * @param out_value Pointer to a bool to store the result.
 * @return true if the key was found and is a boolean, false otherwise.
 */
bool yini_get_bool(YINI_HANDLE handle, const char* section, const char* key, bool* out_value);

/**
 * @brief Retrieves a string value.
 * @param handle The YINI handle.
 * @param section The name of the section.
 * @param key The name of the key.
 * @param out_buffer A buffer to copy the string into.
 * @param buffer_size The size of the provided buffer.
 * @return The number of bytes written to the buffer (excluding null terminator), or -1 if the key is not found or the buffer is too small.
 */
int yini_get_string(YINI_HANDLE handle, const char* section, const char* key, char* out_buffer, int buffer_size);

#ifdef __cplusplus
}
#endif

#endif // YINI_C_API_H
