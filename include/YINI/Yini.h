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

// Opaque handles
typedef struct YiniDocumentHandle YiniDocumentHandle;
typedef struct YiniSectionHandle YiniSectionHandle;
typedef struct YiniValueHandle YiniValueHandle;

typedef enum {
    YINI_TYPE_NONE,
    YINI_TYPE_STRING,
    YINI_TYPE_INT,
    YINI_TYPE_DOUBLE,
    YINI_TYPE_BOOL,
    YINI_TYPE_ARRAY,
    YINI_TYPE_LIST,
    YINI_TYPE_MAP,
    YINI_TYPE_DYNA,
    YINI_TYPE_COORD,
    YINI_TYPE_COLOR,
    YINI_TYPE_PATH
} YiniType;

// Document API
YINI_API YiniDocumentHandle* yini_parse(const char* content, char* error_buffer, int buffer_size);
YINI_API void yini_free_document(YiniDocumentHandle* handle);
YINI_API int yini_get_section_count(const YiniDocumentHandle* handle);
YINI_API const YiniSectionHandle* yini_get_section_by_index(const YiniDocumentHandle* handle, int index);
YINI_API const YiniSectionHandle* yini_get_section_by_name(const YiniDocumentHandle* handle, const char* name);
YINI_API void yini_set_string_value(YiniDocumentHandle* handle, const char* section, const char* key, const char* value);
YINI_API void yini_set_int_value(YiniDocumentHandle* handle, const char* section, const char* key, int value);
YINI_API void yini_set_double_value(YiniDocumentHandle* handle, const char* section, const char* key, double value);
YINI_API void yini_set_bool_value(YiniDocumentHandle* handle, const char* section, const char* key, bool value);

// Section API
YINI_API int yini_section_get_name(const YiniSectionHandle* section_handle, char* buffer, int buffer_size);
YINI_API int yini_section_get_pair_count(const YiniSectionHandle* section_handle);
YINI_API int yini_section_get_pair_key_by_index(const YiniSectionHandle* section_handle, int index, char* buffer, int buffer_size);
YINI_API const YiniValueHandle* yini_section_get_value_by_key(const YiniSectionHandle* section_handle, const char* key);

// Value API
YINI_API YiniType yini_value_get_type(const YiniValueHandle* value_handle);
YINI_API int yini_value_get_string(const YiniValueHandle* value_handle, char* buffer, int buffer_size);
YINI_API int yini_value_get_int(const YiniValueHandle* value_handle);
YINI_API double yini_value_get_double(const YiniValueHandle* value_handle);
YINI_API bool yini_value_get_bool(const YiniValueHandle* value_handle);
YINI_API void yini_value_get_coord(const YiniValueHandle* value_handle, double* x, double* y, double* z, bool* is_3d);
YINI_API void yini_value_get_color(const YiniValueHandle* value_handle, unsigned char* r, unsigned char* g, unsigned char* b);
YINI_API int yini_value_get_path(const YiniValueHandle* value_handle, char* buffer, int buffer_size);
YINI_API int yini_array_get_size(const YiniValueHandle* value_handle);
YINI_API const YiniValueHandle* yini_array_get_value_by_index(const YiniValueHandle* value_handle, int index);

// List API
YINI_API int yini_list_get_size(const YiniValueHandle* value_handle);
YINI_API const YiniValueHandle* yini_list_get_value_by_index(const YiniValueHandle* value_handle, int index);

#ifdef __cplusplus
}
#endif

#endif // YINI_C_API_H