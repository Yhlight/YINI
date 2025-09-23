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
 * @brief Loads a YINI configuration from a .ymeta file.
 * @param filepath Path to the .ymeta file.
 * @return A handle to the YINI runtime, or NULL if loading fails.
 */
YINI_HANDLE yini_load_from_file(const char* filepath);

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


// --- Error Handling Functions ---

/**
 * @brief Gets the total number of errors (parsing and runtime) that have occurred.
 * @param handle The YINI handle.
 * @return The number of errors.
 */
int yini_get_error_count(YINI_HANDLE handle);

/**
 * @brief Gets the details of a specific error.
 * @param handle The YINI handle.
 * @param index The index of the error (from 0 to count-1).
 * @param out_buffer A buffer to copy the error message into.
 * @param buffer_size The size of the provided buffer.
 * @param out_line Pointer to an int to store the line number.
 * @param out_column Pointer to an int to store the column number.
 * @return true if the error index is valid, false otherwise.
 */
bool yini_get_error_details(YINI_HANDLE handle, int index, char* out_buffer, int buffer_size, int* out_line, int* out_column);


// --- Value Modification Functions ---

/**
 * @brief Sets an integer value. If the key does not exist, it will be created.
 * @param handle The YINI handle.
 * @param section The name of the section.
 * @param key The name of the key.
 * @param value The new integer value.
 * @return true on success.
 */
bool yini_set_integer(YINI_HANDLE handle, const char* section, const char* key, long long value);

// TODO: Add yini_set_float, yini_set_bool, yini_set_string


// --- Persistence ---

/**
 * @brief Saves the current state (including modifications to Dyna values) to a .ymeta file.
 * @param handle The YINI handle.
 * @param filepath The path to the output .ymeta file.
 * @return true on success.
 */
bool yini_save_to_file(YINI_HANDLE handle, const char* filepath);


#ifdef __cplusplus
}
#endif

#endif // YINI_C_API_H
