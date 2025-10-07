#ifndef YINI_C_API_H
#define YINI_C_API_H

#ifdef _WIN32
    #ifdef YINI_C_API_EXPORTS
        #define YINI_C_API __declspec(dllexport)
    #else
        #define YINI_C_API __declspec(dllimport)
    #endif
#else
    #define YINI_C_API
#endif

extern "C" {
    YINI_C_API void* yini_load(const char* filepath);
    YINI_C_API void yini_free(void* handle);
    YINI_C_API const char* yini_get_string(void* handle, const char* section, const char* key);
    YINI_C_API void yini_free_string(const char* str);
    YINI_C_API int yini_get_int(void* handle, const char* section, const char* key);
    YINI_C_API double yini_get_double(void* handle, const char* section, const char* key);
    YINI_C_API bool yini_get_bool(void* handle, const char* section, const char* key);
}

#endif // YINI_C_API_H