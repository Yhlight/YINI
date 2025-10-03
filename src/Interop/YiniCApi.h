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

    // Opaque handles
    typedef void* Yini_ManagerHandle;
    typedef void* Yini_ValueHandle;

    // Enum to represent the type of a YiniValue
    typedef enum {
        YINI_TYPE_NULL,
        YINI_TYPE_BOOL,
        YINI_TYPE_DOUBLE,
        YINI_TYPE_STRING,
        YINI_TYPE_ARRAY,
        YINI_TYPE_MAP,
        YINI_TYPE_DYNA
    } Yini_ValueType;

    // --- Manager Functions ---
    YINI_API Yini_ManagerHandle yini_manager_create();
    YINI_API void yini_manager_destroy(Yini_ManagerHandle manager);
    YINI_API bool yini_manager_load(Yini_ManagerHandle manager, const char* filepath);
    YINI_API void yini_manager_save_changes(Yini_ManagerHandle manager);

    // --- Value Get/Set on Manager ---
    YINI_API Yini_ValueHandle yini_manager_get_value(Yini_ManagerHandle manager, const char* section, const char* key);
    YINI_API void yini_manager_set_value(Yini_ManagerHandle manager, const char* section, const char* key, Yini_ValueHandle value_handle);

    // --- Value Handle Functions ---
    YINI_API void yini_value_destroy(Yini_ValueHandle handle);
    YINI_API Yini_ValueType yini_value_get_type(Yini_ValueHandle handle);

    // --- Create Value Handles ---
    YINI_API Yini_ValueHandle yini_value_create_double(double value);
    YINI_API Yini_ValueHandle yini_value_create_string(const char* value);
    YINI_API Yini_ValueHandle yini_value_create_bool(bool value);
    YINI_API Yini_ValueHandle yini_value_create_array();
    YINI_API Yini_ValueHandle yini_value_create_map();

    // --- Get Data from Value Handles ---
    YINI_API bool yini_value_get_double(Yini_ValueHandle handle, double* out_value);
    YINI_API int yini_value_get_string(Yini_ValueHandle handle, char* out_buffer, int buffer_size);
    YINI_API bool yini_value_get_bool(Yini_ValueHandle handle, bool* out_value);
    YINI_API Yini_ValueHandle yini_value_get_dyna_value(Yini_ValueHandle handle);

    // --- Array Manipulation ---
    YINI_API int yini_array_get_size(Yini_ValueHandle handle);
    YINI_API Yini_ValueHandle yini_array_get_element(Yini_ValueHandle handle, int index);
    YINI_API void yini_array_add_element(Yini_ValueHandle array_handle, Yini_ValueHandle element_handle);

    // --- Map Manipulation ---
    YINI_API int yini_map_get_size(Yini_ValueHandle handle);
    YINI_API Yini_ValueHandle yini_map_get_value_at(Yini_ValueHandle handle, int index);
    YINI_API const char* yini_map_get_key_at(Yini_ValueHandle handle, int index);
    YINI_API void yini_map_set_value(Yini_ValueHandle map_handle, const char* key, Yini_ValueHandle value_handle);


#ifdef __cplusplus
}
#endif

#endif // YINI_C_API_H