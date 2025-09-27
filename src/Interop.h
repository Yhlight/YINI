#pragma once

// Define platform-specific export macros
#if defined(_WIN32)
    #define YINI_API __declspec(dllexport)
#else
    #define YINI_API __attribute__((visibility("default")))
#endif

// Use extern "C" to prevent C++ name mangling, which is crucial for P/Invoke
#ifdef __cplusplus
extern "C" {
#endif

YINI_API int add(int a, int b);

#ifdef __cplusplus
}
#endif