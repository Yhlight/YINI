#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Opaque handle to a YINI document context
struct YiniDocument;
typedef struct YiniDocument* YiniDocumentHandle;

// Error codes
typedef enum {
    YINI_OK = 0,
    YINI_ERROR_FILE_NOT_FOUND,
    YINI_ERROR_PARSE_ERROR,
    YINI_ERROR_KEY_NOT_FOUND,
    YINI_ERROR_TYPE_MISMATCH,
    YINI_ERROR_EVALUATION_ERROR,
    YINI_ERROR_UNKNOWN
} YiniResult;

// --- API Functions ---

/**
 * @brief Loads a YINI file from the specified path, parses, and resolves it.
 *
 * @param filepath The path to the .yini file.
 * @param out_handle A pointer to a handle that will be populated with the loaded document.
 * @return YiniResult A result code indicating success or failure.
 */
YiniResult Yini_LoadFromFile(const char* filepath, YiniDocumentHandle* out_handle);

/**
 * @brief Frees all resources associated with a YINI document.
 *
 * @param handle The handle to the document to free.
 */
void Yini_Free(YiniDocumentHandle handle);

/**
 * @brief Retrieves an integer value from the document.
 *
 * The key should be in the format "Section.Key". The value will be evaluated
 * if it is an expression.
 *
 * @param handle The document handle.
 * @param key The key to look up (e.g., "PlayerStats.attack").
 * @param out_value A pointer to an integer that will be populated with the result.
 * @return YiniResult A result code indicating success or failure.
 */
YiniResult Yini_GetValue_Int(YiniDocumentHandle handle, const char* key, int64_t* out_value);

#ifdef __cplusplus
}
#endif
