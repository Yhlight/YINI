#ifndef YINI_INTEROP_H
#define YINI_INTEROP_H

#include <stddef.h>
#include <stdbool.h>

#ifdef _WIN32
    #define YINI_API __declspec(dllexport)
#else
    #define YINI_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef void* YiniConfigHandle;

YINI_API YiniConfigHandle yini_parse_file(const char* filepath);

YINI_API const char* yini_get_string(YiniConfigHandle handle, const char* section, const char* key);
YINI_API int yini_get_int(YiniConfigHandle handle, const char* section, const char* key, int default_value);
YINI_API double yini_get_double(YiniConfigHandle handle, const char* section, const char* key, double default_value);
YINI_API bool yini_get_bool(YiniConfigHandle handle, const char* section, const char* key, bool default_value);

YINI_API void yini_free_config(YiniConfigHandle handle);

#ifdef __cplusplus
}
#endif

#endif // YINI_INTEROP_H