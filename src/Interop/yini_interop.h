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

typedef enum {
    YINI_TYPE_NULL,
    YINI_TYPE_STRING,
    YINI_TYPE_INT,
    YINI_TYPE_DOUBLE,
    YINI_TYPE_BOOL,
    YINI_TYPE_ARRAY,
    YINI_TYPE_MAP
} YiniValueType;

// Forward declarations
typedef struct YiniValue YiniValue;
typedef struct YiniArray YiniArray;
typedef struct YiniMap YiniMap;

struct YiniValue {
    YiniValueType type;
    union {
        const char* string_value;
        int int_value;
        double double_value;
        bool bool_value;
        YiniArray* array_value;
        YiniMap* map_value;
    } as;
};

struct YiniArray {
    size_t size;
    YiniValue** elements;
};

typedef struct {
    const char* key;
    YiniValue* value;
} YiniMapEntry;

struct YiniMap {
    size_t size;
    YiniMapEntry** entries;
};

YINI_API YiniConfigHandle yini_parse_file(const char* filepath);

YINI_API const char* yini_get_string(YiniConfigHandle handle, const char* section, const char* key);
YINI_API int yini_get_int(YiniConfigHandle handle, const char* section, const char* key, int default_value);
YINI_API double yini_get_double(YiniConfigHandle handle, const char* section, const char* key, double default_value);
YINI_API bool yini_get_bool(YiniConfigHandle handle, const char* section, const char* key, bool default_value);

YINI_API YiniValue* yini_get_value(YiniConfigHandle handle, const char* section, const char* key);
YINI_API void yini_free_value(YiniValue* value);

typedef struct {
    YiniValue* value;
    YiniArray* backups;
} YiniDynaValue;

YINI_API YiniDynaValue* yini_get_dyna(YiniConfigHandle handle, const char* section, const char* key);
YINI_API void yini_free_dyna(YiniDynaValue* dyna);

YINI_API void yini_set_string(YiniConfigHandle handle, const char* section, const char* key, const char* value);
YINI_API void yini_set_int(YiniConfigHandle handle, const char* section, const char* key, int value);
YINI_API void yini_set_double(YiniConfigHandle handle, const char* section, const char* key, double value);
YINI_API void yini_set_bool(YiniConfigHandle handle, const char* section, const char* key, bool value);

YINI_API void yini_save_file(YiniConfigHandle handle, const char* filepath);

YINI_API void yini_free_config(YiniConfigHandle handle);

#ifdef __cplusplus
}
#endif

#endif // YINI_INTEROP_H