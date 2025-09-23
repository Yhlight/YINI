#include "../include/yini.h"
#include "c_api_internal.h"
#include "../include/yini.h"
#include <string>
#include <vector>
#include <cstring> // For strncpy

static YiniHandleInternal* toHandle(YINI_HANDLE handle)
{
    return static_cast<YiniHandleInternal*>(handle);
}

YINI_HANDLE yini_load_from_string(const char* content)
{
    if (!content) return nullptr;
    // Loading from string does not support #include, so we can't use the parser
    // as part of the handle for unified error reporting. This is a limitation.
    // We will create a dummy parser.
    auto* runtime = new Yini::YiniRuntime();
    runtime->loadFromString(content);
    auto* handle_internal = new YiniHandleInternal{nullptr, nullptr, runtime};
    return handle_internal;
}

YINI_HANDLE yini_load_from_file(const char* filepath)
{
    if (!filepath) return nullptr;
    auto* runtime = new Yini::YiniRuntime();
    if (runtime->loadFromFile(filepath))
    {
        // For file loading, we don't have access to the parser's errors with this design.
        // This is a known limitation that would require a deeper refactor.
        auto* handle_internal = new YiniHandleInternal{nullptr, nullptr, runtime};
        return handle_internal;
    }
    delete runtime;
    return nullptr;
}

void yini_free(YINI_HANDLE handle)
{
    if (handle)
    {
        auto* h = toHandle(handle);
        delete h->lexer; // This is now safe
        delete h->parser;
        delete h->runtime;
        delete h;
    }
}

// --- Error Handling C API Implementation ---

int yini_get_error_count(YINI_HANDLE handle)
{
    if (!handle) return 0;
    auto* h = toHandle(handle);
    int count = 0;
    if (h->parser) count += h->parser->getErrors().size();
    if (h->runtime) count += h->runtime->getErrors().size();
    return count;
}

bool yini_get_error_details(YINI_HANDLE handle, int index, char* out_buffer, int buffer_size, int* out_line, int* out_column)
{
    if (!handle || !out_buffer || buffer_size <= 0 || index < 0) return false;
    auto* h = toHandle(handle);

    int parser_error_count = h->parser ? h->parser->getErrors().size() : 0;

    const Yini::YiniError* err = nullptr;

    if (index < parser_error_count) {
        err = &h->parser->getErrors()[index];
    } else if (h->runtime) {
        int runtime_index = index - parser_error_count;
        if (runtime_index < (int)h->runtime->getErrors().size()) {
            err = &h->runtime->getErrors()[runtime_index];
        }
    }

    if (!err) return false;

    if (out_line) *out_line = err->line;
    if (out_column) *out_column = err->column;

    #ifdef _WIN32
        strcpy_s(out_buffer, buffer_size, err->message.c_str());
    #else
        strncpy(out_buffer, err->message.c_str(), buffer_size - 1);
        out_buffer[buffer_size - 1] = '\0';
    #endif

    return true;
}

bool yini_set_integer(YINI_HANDLE handle, const char* section, const char* key, long long value)
{
    auto* h = toHandle(handle);
    if (!h || !h->runtime || !section || !key) return false;
    auto val = std::make_shared<Yini::Value>();
    val->data.emplace<Yini::Integer>(value);
    return h->runtime->setValue(section, key, val);
}

bool yini_save_to_file(YINI_HANDLE handle, const char* filepath)
{
    auto* h = toHandle(handle);
    if (!h || !h->runtime || !filepath) return false;
    return h->runtime->serialize(filepath);
}

bool yini_get_integer(YINI_HANDLE handle, const char* section, const char* key, long long* out_value)
{
    auto* h = toHandle(handle);
    if (!h || !h->runtime || !section || !key || !out_value) return false;

    auto value = h->runtime->getValue(section, key);

    if (value && std::holds_alternative<Yini::Integer>(value->data))
    {
        *out_value = std::get<Yini::Integer>(value->data);
        return true;
    }
    return false;
}

bool yini_get_float(YINI_HANDLE handle, const char* section, const char* key, double* out_value)
{
    auto* h = toHandle(handle);
    if (!h || !h->runtime || !section || !key || !out_value) return false;

    auto value = h->runtime->getValue(section, key);

    if (!value) return false;

    if (std::holds_alternative<Yini::Float>(value->data))
    {
        *out_value = std::get<Yini::Float>(value->data);
        return true;
    }
    if (std::holds_alternative<Yini::Integer>(value->data))
    {
        *out_value = static_cast<double>(std::get<Yini::Integer>(value->data));
        return true;
    }
    return false;
}

bool yini_get_bool(YINI_HANDLE handle, const char* section, const char* key, bool* out_value)
{
    auto* h = toHandle(handle);
    if (!h || !h->runtime || !section || !key || !out_value) return false;

    auto value = h->runtime->getValue(section, key);

    if (value && std::holds_alternative<Yini::Boolean>(value->data))
    {
        *out_value = std::get<Yini::Boolean>(value->data);
        return true;
    }
    return false;
}

int yini_get_string(YINI_HANDLE handle, const char* section, const char* key, char* out_buffer, int buffer_size)
{
    auto* h = toHandle(handle);
    if (!h || !h->runtime || !section || !key || !out_buffer || buffer_size <= 0) return -1;

    auto value = h->runtime->getValue(section, key);

    if (value && std::holds_alternative<Yini::String>(value->data))
    {
        const auto& str = std::get<Yini::String>(value->data);
        if ((int)str.length() + 1 > buffer_size)
        {
            return -1;
        }
        #ifdef _WIN32
            strcpy_s(out_buffer, buffer_size, str.c_str());
        #else
            strncpy(out_buffer, str.c_str(), buffer_size - 1);
            out_buffer[buffer_size - 1] = '\0';
        #endif
        return static_cast<int>(str.length());
    }
    return -1;
}
