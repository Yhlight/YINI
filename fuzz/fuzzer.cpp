#include "YINI/Yini.h"
#include <cstdint>
#include <cstddef>

// The entry point for the libFuzzer engine.
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    // Create a null-terminated string from the fuzzer's input data.
    std::string content(reinterpret_cast<const char*>(Data), Size);

    // Create a buffer for potential error messages, though we won't inspect it.
    char error_buffer[256];

    // Call the function we want to test.
    YiniDocumentHandle* doc = yini_parse(content.c_str(), error_buffer, sizeof(error_buffer));

    // If the parse succeeded, we must free the document to avoid memory leaks.
    if (doc) {
        yini_free_document(doc);
    }

    // The return value is always 0 for libFuzzer harnesses.
    return 0;
}