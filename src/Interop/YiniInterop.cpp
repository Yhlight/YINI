#include "YiniInterop.h"
#include "../Parser/YiniLoader.h"
#include "../Parser/Ast.h"
#include "../Json/Json.h"

#include <string>
#include <variant>
#include <iostream>
#include <cstring>

// --- Opaque Handle Definitions ---
// The C-style handles are just empty structs used for type-safe pointers
// to the internal C++ AST nodes.
struct YiniHandle { YiniFile ast; };
struct YiniValueHandle {};
struct YiniArrayHandle {};
struct YiniObjectHandle {};


// --- Helper function to safely get a value from the AST ---
const YiniValue* find_value(YiniHandle* handle, const char* section, const char* key)
{
    if (!handle || !section || !key)
    {
        return nullptr;
    }
    auto& ast = handle->ast;
    auto sec_it = ast.sectionsMap.find(section);
    if (sec_it == ast.sectionsMap.end())
    {
        return nullptr;
    }
    auto& key_values = sec_it->second.keyValues;
    auto key_it = key_values.find(key);
    if (key_it == key_values.end())
    {
        return nullptr;
    }
    return &key_it->second;
}


// --- API Implementations ---
extern "C" {

YINI_API YiniHandle* yini_load(const char* filepath)
{
    try
    {
        Yini::Loader loader;
        YiniFile ast = loader.load(filepath);
        YiniHandle* handle = new YiniHandle{std::move(ast)};
        return handle;
    }
    catch (const std::exception& e)
    {
        std::cerr << "YINI load error: " << e.what() << std::endl;
        return nullptr;
    }
}

YINI_API void yini_free(YiniHandle* handle)
{
    if (handle)
    {
        delete handle;
    }
}

// --- Simple Getters ---
YINI_API int yini_get_string(YiniHandle* handle, const char* section, const char* key, char* out_buffer, int buffer_size)
{
    const YiniValue* yini_value = find_value(handle, section, key);
    if (!yini_value || !std::holds_alternative<std::string>(yini_value->value) || !out_buffer || buffer_size <= 0)
    {
        return -1;
    }
    const std::string& str = std::get<std::string>(yini_value->value);
    int bytes_to_copy = std::min((int)str.length(), buffer_size - 1);
    memcpy(out_buffer, str.c_str(), bytes_to_copy);
    out_buffer[bytes_to_copy] = '\0';
    return bytes_to_copy;
}

YINI_API int yini_get_int64(YiniHandle* handle, const char* section, const char* key, int64_t* out_value)
{
    const YiniValue* yini_value = find_value(handle, section, key);
    if (!yini_value || !std::holds_alternative<int64_t>(yini_value->value) || !out_value) return 0;
    *out_value = std::get<int64_t>(yini_value->value);
    return 1;
}

YINI_API int yini_get_double(YiniHandle* handle, const char* section, const char* key, double* out_value)
{
    const YiniValue* yini_value = find_value(handle, section, key);
    if (!yini_value || !out_value) return 0;
    if (std::holds_alternative<double>(yini_value->value))
    {
        *out_value = std::get<double>(yini_value->value);
        return 1;
    }
    if (std::holds_alternative<int64_t>(yini_value->value))
    {
        *out_value = static_cast<double>(std::get<int64_t>(yini_value->value));
        return 1;
    }
    return 0;
}

YINI_API int yini_get_bool(YiniHandle* handle, const char* section, const char* key, bool* out_value)
{
    const YiniValue* yini_value = find_value(handle, section, key);
    if (!yini_value || !std::holds_alternative<bool>(yini_value->value) || !out_value) return 0;
    *out_value = std::get<bool>(yini_value->value);
    return 1;
}

YINI_API int yini_get_value_as_json(YiniHandle* handle, const char* section, const char* key, char* out_buffer, int buffer_size)
{
    const YiniValue* yini_value = find_value(handle, section, key);
    if (!yini_value) return -1;
    std::string json_str = Yini::JsonWriter::write(*yini_value);
    if (!out_buffer || buffer_size <= 0) return -1;
    int bytes_to_copy = std::min((int)json_str.length(), buffer_size - 1);
    memcpy(out_buffer, json_str.c_str(), bytes_to_copy);
    out_buffer[bytes_to_copy] = '\0';
    return bytes_to_copy;
}

// --- Granular API Implementations ---

YINI_API YiniValueHandle* yini_get_value(YiniHandle* handle, const char* section, const char* key)
{
    return reinterpret_cast<YiniValueHandle*>(const_cast<YiniValue*>(find_value(handle, section, key)));
}

YINI_API Yini_Value_Type yini_value_get_type(YiniValueHandle* value_handle)
{
    if (!value_handle) return YINI_TYPE_UNINITIALIZED;
    auto* val = reinterpret_cast<YiniValue*>(value_handle);
    if (std::holds_alternative<std::string>(val->value)) return YINI_TYPE_STRING;
    if (std::holds_alternative<int64_t>(val->value)) return YINI_TYPE_INT64;
    if (std::holds_alternative<double>(val->value)) return YINI_TYPE_DOUBLE;
    if (std::holds_alternative<bool>(val->value)) return YINI_TYPE_BOOL;
    if (std::holds_alternative<YiniArray>(val->value)) return YINI_TYPE_ARRAY;
    if (std::holds_alternative<YiniCoord>(val->value)) return YINI_TYPE_COORD;
    if (std::holds_alternative<YiniColor>(val->value)) return YINI_TYPE_COLOR;
    if (std::holds_alternative<YiniObject>(val->value)) return YINI_TYPE_OBJECT;
    return YINI_TYPE_UNINITIALIZED;
}

YINI_API int yini_value_as_string(YiniValueHandle* value_handle, char* out_buffer, int buffer_size)
{
    if (!value_handle || !std::holds_alternative<std::string>(reinterpret_cast<YiniValue*>(value_handle)->value) || !out_buffer || buffer_size <= 0) return -1;
    const std::string& str = std::get<std::string>(reinterpret_cast<YiniValue*>(value_handle)->value);
    int bytes_to_copy = std::min((int)str.length(), buffer_size - 1);
    memcpy(out_buffer, str.c_str(), bytes_to_copy);
    out_buffer[bytes_to_copy] = '\0';
    return bytes_to_copy;
}

YINI_API int yini_value_as_int64(YiniValueHandle* value_handle, int64_t* out_value)
{
    if (!value_handle || !std::holds_alternative<int64_t>(reinterpret_cast<YiniValue*>(value_handle)->value) || !out_value) return 0;
    *out_value = std::get<int64_t>(reinterpret_cast<YiniValue*>(value_handle)->value);
    return 1;
}

YINI_API int yini_value_as_double(YiniValueHandle* value_handle, double* out_value)
{
    if (!value_handle || !out_value) return 0;
    YiniValue* val = reinterpret_cast<YiniValue*>(value_handle);
    if (std::holds_alternative<double>(val->value))
    {
        *out_value = std::get<double>(val->value);
        return 1;
    }
    if (std::holds_alternative<int64_t>(val->value))
    {
        *out_value = static_cast<double>(std::get<int64_t>(val->value));
        return 1;
    }
    return 0;
}

YINI_API int yini_value_as_bool(YiniValueHandle* value_handle, bool* out_value)
{
    if (!value_handle || !std::holds_alternative<bool>(reinterpret_cast<YiniValue*>(value_handle)->value) || !out_value) return 0;
    *out_value = std::get<bool>(reinterpret_cast<YiniValue*>(value_handle)->value);
    return 1;
}

YINI_API YiniArrayHandle* yini_value_as_array(YiniValueHandle* value_handle)
{
    if (!value_handle || !std::holds_alternative<YiniArray>(reinterpret_cast<YiniValue*>(value_handle)->value)) return nullptr;
    return reinterpret_cast<YiniArrayHandle*>(&std::get<YiniArray>(reinterpret_cast<YiniValue*>(value_handle)->value));
}

YINI_API YiniObjectHandle* yini_value_as_object(YiniValueHandle* value_handle)
{
    if (!value_handle || !std::holds_alternative<YiniObject>(reinterpret_cast<YiniValue*>(value_handle)->value)) return nullptr;
    return reinterpret_cast<YiniObjectHandle*>(&std::get<YiniObject>(reinterpret_cast<YiniValue*>(value_handle)->value));
}

// Array Access
YINI_API int yini_array_get_size(YiniArrayHandle* array_handle)
{
    if (!array_handle) return -1;
    return static_cast<int>(reinterpret_cast<YiniArray*>(array_handle)->size());
}

YINI_API YiniValueHandle* yini_array_get_value(YiniArrayHandle* array_handle, int index)
{
    if (!array_handle) return nullptr;
    auto* arr = reinterpret_cast<YiniArray*>(array_handle);
    if (index < 0 || static_cast<size_t>(index) >= arr->size()) return nullptr;
    return reinterpret_cast<YiniValueHandle*>(&(*arr)[index]);
}

YINI_API int yini_value_as_json(YiniValueHandle* value_handle, char* out_buffer, int buffer_size)
{
    if (!value_handle) return -1;
    auto* yini_value = reinterpret_cast<YiniValue*>(value_handle);

    std::string json_str = Yini::JsonWriter::write(*yini_value);

    if (!out_buffer || buffer_size <= 0) return -1;

    int bytes_to_copy = std::min((int)json_str.length(), buffer_size - 1);
    memcpy(out_buffer, json_str.c_str(), bytes_to_copy);
    out_buffer[bytes_to_copy] = '\0';

    return bytes_to_copy;
}

} // extern "C"
