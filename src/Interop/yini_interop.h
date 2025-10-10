#pragma once

#ifdef _WIN32
    #define YINI_API __declspec(dllexport)
#else
    #define YINI_API __attribute__((visibility("default")))
#endif

extern "C" {

    YINI_API void* yini_create_from_file(const char* file_path);
    YINI_API void yini_destroy(void* handle);

    YINI_API bool yini_get_int(void* handle, const char* key, int* out_value);
    YINI_API bool yini_get_double(void* handle, const char* key, double* out_value);
    YINI_API bool yini_get_bool(void* handle, const char* key, bool* out_value);
    YINI_API const char* yini_get_string(void* handle, const char* key);
    YINI_API void yini_free_string(const char* str);

}