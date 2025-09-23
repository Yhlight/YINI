#include "c_api_internal.h"
#include "../include/yini.h"
#include <string>
#include <vector>
#include <cstring>

static YiniHandleInternal* toHandle(YINI_HANDLE handle)
{
    return static_cast<YiniHandleInternal*>(handle);
}

YINI_HANDLE yini_load_from_string(const char* content)
{
    if (!content) return nullptr;

    auto* runtime = new Yini::YiniRuntime();
    auto* handle_internal = new YiniHandleInternal{runtime, {}};

    try {
        runtime->loadFromString(content);
        handle_internal->aggregated_errors = runtime->getErrors();
    } catch (const std::exception& e) {
        handle_internal->aggregated_errors.emplace_back(Yini::ErrorType::Runtime, e.what());
    }

    return handle_internal;
}

YINI_HANDLE yini_load_from_file(const char* filepath)
{
    if (!filepath) return nullptr;
    auto* runtime = new Yini::YiniRuntime();
    auto* handle_internal = new YiniHandleInternal{runtime, {}};

    if (!runtime->loadFromFile(filepath)) {
        handle_internal->aggregated_errors = runtime->getErrors();
    }

    return handle_internal;
}

void yini_free(YINI_HANDLE handle)
{
    if (handle) {
        auto* h = toHandle(handle);
        delete h->runtime;
        delete h;
    }
}

int yini_get_error_count(YINI_HANDLE handle)
{
    if (!handle) return 0;
    return static_cast<int>(toHandle(handle)->aggregated_errors.size());
}

bool yini_get_error_details(YINI_HANDLE handle, int index, char* out_buffer, int buffer_size, int* out_line, int* out_column)
{
    if (!handle || !out_buffer || buffer_size <= 0 || index < 0) return false;
    auto* h = toHandle(handle);
    if (index >= (int)h->aggregated_errors.size()) return false;
    const auto& err = h->aggregated_errors[index];
    if (out_line) *out_line = err.line;
    if (out_column) *out_column = err.column;
    strncpy(out_buffer, err.message.c_str(), buffer_size - 1);
    out_buffer[buffer_size - 1] = '\0';
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
    // return h->runtime->serialize(filepath); // serialize is not yet implemented
    return false;
}

bool yini_get_integer(YINI_HANDLE handle, const char* section, const char* key, long long* out_value)
{
    auto* h = toHandle(handle);
    if (!h || !h->runtime || !section || !key || !out_value) return false;
    auto value = h->runtime->getValue(section, key);
    if (value && std::holds_alternative<Yini::Integer>(value->data)) {
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
    if (std::holds_alternative<Yini::Float>(value->data)) {
        *out_value = std::get<Yini::Float>(value->data);
        return true;
    }
    if (std::holds_alternative<Yini::Integer>(value->data)) {
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
    if (value && std::holds_alternative<Yini::Boolean>(value->data)) {
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
    if (value && std::holds_alternative<Yini::String>(value->data)) {
        const auto& str = std::get<Yini::String>(value->data);
        if ((int)str.length() + 1 > buffer_size) return -1;
        strncpy(out_buffer, str.c_str(), buffer_size - 1);
        out_buffer[buffer_size - 1] = '\0';
        return static_cast<int>(str.length());
    }
    return -1;
}
