#include "../include/yini.h"
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "Runtime/Runtime.h"
#include <string>
#include <vector>
#include <cstring> // For strncpy

// Helper to convert handle to runtime pointer
static Yini::YiniRuntime* toRuntime(YINI_HANDLE handle)
{
    return static_cast<Yini::YiniRuntime*>(handle);
}

YINI_HANDLE yini_load_from_string(const char* content)
{
    if (!content)
    {
        return nullptr;
    }

    // Perform parsing and evaluation
    Yini::Lexer lexer(content);
    Yini::Parser parser(lexer);
    auto doc = parser.parseDocument();

    // Check for parsing errors
    if (!parser.getErrors().empty())
    {
        // In a real scenario, we might have an error reporting mechanism here.
        return nullptr;
    }

    // The runtime will be allocated on the heap and its pointer returned as a handle.
    auto* runtime = new Yini::YiniRuntime();
    runtime->evaluate(doc.get());

    return runtime;
}

void yini_free(YINI_HANDLE handle)
{
    if (handle)
    {
        delete toRuntime(handle);
    }
}

bool yini_get_integer(YINI_HANDLE handle, const char* section, const char* key, long long* out_value)
{
    if (!handle || !section || !key || !out_value) return false;

    auto runtime = toRuntime(handle);
    auto value = runtime->getValue(section, key);

    if (value && std::holds_alternative<Yini::Integer>(value->data))
    {
        *out_value = std::get<Yini::Integer>(value->data);
        return true;
    }
    return false;
}

bool yini_get_float(YINI_HANDLE handle, const char* section, const char* key, double* out_value)
{
    if (!handle || !section || !key || !out_value) return false;

    auto runtime = toRuntime(handle);
    auto value = runtime->getValue(section, key);

    if (!value) return false;

    if (std::holds_alternative<Yini::Float>(value->data))
    {
        *out_value = std::get<Yini::Float>(value->data);
        return true;
    }
    // Allow implicit conversion from Integer to Float
    if (std::holds_alternative<Yini::Integer>(value->data))
    {
        *out_value = static_cast<double>(std::get<Yini::Integer>(value->data));
        return true;
    }
    return false;
}

bool yini_get_bool(YINI_HANDLE handle, const char* section, const char* key, bool* out_value)
{
    if (!handle || !section || !key || !out_value) return false;

    auto runtime = toRuntime(handle);
    auto value = runtime->getValue(section, key);

    if (value && std::holds_alternative<Yini::Boolean>(value->data))
    {
        *out_value = std::get<Yini::Boolean>(value->data);
        return true;
    }
    return false;
}

int yini_get_string(YINI_HANDLE handle, const char* section, const char* key, char* out_buffer, int buffer_size)
{
    if (!handle || !section || !key || !out_buffer || buffer_size <= 0) return -1;

    auto runtime = toRuntime(handle);
    auto value = runtime->getValue(section, key);

    if (value && std::holds_alternative<Yini::String>(value->data))
    {
        const auto& str = std::get<Yini::String>(value->data);
        if ((int)str.length() + 1 > buffer_size)
        {
            return -1; // Buffer too small
        }
        // Copy string and null terminator
        #ifdef _WIN32
            strcpy_s(out_buffer, buffer_size, str.c_str());
        #else
            strncpy(out_buffer, str.c_str(), buffer_size - 1);
            out_buffer[buffer_size - 1] = '\0'; // ensure null termination
        #endif
        return static_cast<int>(str.length());
    }
    return -1;
}
