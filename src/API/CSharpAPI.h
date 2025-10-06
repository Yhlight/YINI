#ifndef YINI_API_CSHARP_API_H
#define YINI_API_CSHARP_API_H

#ifdef _WIN32
    #define YINI_EXPORT __declspec(dllexport)
#else
    #define YINI_EXPORT __attribute__((visibility("default")))
#endif

extern "C"
{
    // C API for P/Invoke (placeholder)
    YINI_EXPORT void* yini_create_lexer(const char* source);
    YINI_EXPORT void yini_destroy_lexer(void* lexer);
    YINI_EXPORT int yini_tokenize(void* lexer);
}

#endif // YINI_API_CSHARP_API_H
