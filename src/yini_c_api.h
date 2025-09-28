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

extern "C" {
    YINI_API void* yini_load_from_string(const char* yini_string);
    YINI_API void yini_free_ast(void* ast_ptr);
    // More functions will be added here to inspect the AST
}

#endif // YINI_C_API_H