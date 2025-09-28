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

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct YiniDocumentHandle YiniDocumentHandle;

YINI_API YiniDocumentHandle* yini_parse(const char* content);
YINI_API void yini_free_document(YiniDocumentHandle* handle);
YINI_API int yini_get_section_count(YiniDocumentHandle* handle);
YINI_API const char* yini_get_section_name(YiniDocumentHandle* handle, int section_index);

#ifdef __cplusplus
}
#endif

#endif // YINI_C_API_H