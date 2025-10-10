#pragma once

#include <cstdint>

#ifdef _WIN32
    #define YINI_API __declspec(dllexport)
#else
    #define YINI_API __attribute__((visibility("default")))
#endif

extern "C" {

    // --- Structs for complex types ---
    struct Yini_Color {
        uint8_t r, g, b;
    };

    struct Yini_Coord {
        double x, y, z;
        bool has_z;
    };

    YINI_API void* yini_create_from_file(const char* file_path);
    YINI_API void yini_destroy(void* handle);

    // --- Primitive Getters ---
    YINI_API bool yini_get_int(void* handle, const char* key, int* out_value);
    YINI_API bool yini_get_double(void* handle, const char* key, double* out_value);
    YINI_API bool yini_get_bool(void* handle, const char* key, bool* out_value);
    YINI_API int yini_get_string_length(void* handle, const char* key);
    YINI_API int yini_get_string(void* handle, const char* key, char* out_buffer, int buffer_size);

    // --- Complex Type Getters ---
    YINI_API bool yini_get_color(void* handle, const char* key, Yini_Color* out_value);
    YINI_API bool yini_get_coord(void* handle, const char* key, Yini_Coord* out_value);

    // --- Collection Getters ---
    YINI_API int yini_get_array_size(void* handle, const char* key);
    YINI_API int yini_get_array_item_as_string_length(void* handle, const char* key, int index);
    YINI_API int yini_get_array_item_as_string(void* handle, const char* key, int index, char* out_buffer, int buffer_size);
}