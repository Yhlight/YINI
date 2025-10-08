#ifndef YINI_API_H
#define YINI_API_H

#ifdef _WIN32
    #define YINI_API __declspec(dllexport)
#else
    #define YINI_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef void* YiniDocumentHandle;

YINI_API YiniDocumentHandle yini_parse_string(const char* source);
YINI_API void yini_free_document(YiniDocumentHandle handle);

#ifdef __cplusplus
}
#endif

#endif // YINI_API_H