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

// --- Opaque handle to the YINI file data ---
// In C++, this will be a pointer to a YiniFile object.
struct YiniHandle;
typedef struct YiniHandle YiniHandle;


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


#ifdef __cplusplus
}
#endif

#endif // YINI_INTEROP_H
