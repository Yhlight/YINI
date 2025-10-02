#ifndef YINI_C_API_H
#define YINI_C_API_H

#ifdef _WIN32
    #define YINI_API __declspec(dllexport)
#else
    #define YINI_API __attribute__((visibility("default")))
#endif

extern "C" {

    YINI_API void* yini_manager_create();
    YINI_API void yini_manager_destroy(void* manager);
    YINI_API bool yini_manager_load(void* manager, const char* filepath);
    YINI_API void yini_manager_save_changes(void* manager);

    YINI_API bool yini_manager_get_double(void* manager, const char* section, const char* key, double* out_value);
    YINI_API int yini_manager_get_string(void* manager, const char* section, const char* key, char* out_buffer, int buffer_size);
    YINI_API bool yini_manager_get_bool(void* manager, const char* section, const char* key, bool* out_value);

    YINI_API void yini_manager_set_double(void* manager, const char* section, const char* key, double value);
    YINI_API void yini_manager_set_string(void* manager, const char* section, const char* key, const char* value);
    YINI_API void yini_manager_set_bool(void* manager, const char* section, const char* key, bool value);

}

#endif // YINI_C_API_H