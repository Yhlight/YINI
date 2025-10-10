#pragma once

#ifdef _WIN32
    #define YINI_API __declspec(dllexport)
#else
    #define YINI_API __attribute__((visibility("default")))
#endif

extern "C" {

    YINI_API void* yini_create_from_file(const char* file_path);
    YINI_API const char* yini_get_last_error();
    YINI_API void yini_destroy(void* handle);

    YINI_API bool yini_get_int(void* handle, const char* key, int* out_value);
    YINI_API bool yini_get_double(void* handle, const char* key, double* out_value);
    YINI_API bool yini_get_bool(void* handle, const char* key, bool* out_value);
    YINI_API const char* yini_get_string(void* handle, const char* key);
    YINI_API void yini_free_string(const char* str);

    // --- Collection Getters ---
    YINI_API int yini_get_array_size(void* handle, const char* key);
    YINI_API bool yini_get_array_item_as_int(void* handle, const char* key, int index, int* out_value);
    YINI_API bool yini_get_array_item_as_double(void* handle, const char* key, int index, double* out_value);
    YINI_API bool yini_get_array_item_as_bool(void* handle, const char* key, int index, bool* out_value);
    YINI_API const char* yini_get_array_item_as_string(void* handle, const char* key, int index);
}