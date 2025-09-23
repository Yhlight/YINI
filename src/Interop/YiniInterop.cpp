#include "YiniInterop.h"
#include "../Parser/YiniLoader.h"
#include "../Parser/Ast.h"

#include <string>
#include <variant>
#include <iostream>
#include <cstring>

// Define the handle to be a YiniFile from the C++ implementation.
// This is the "opaque pointer" pattern. The C consumer of the API
// only knows about `YiniHandle`, not its internal structure.
struct YiniHandle
{
    YiniFile ast;
};

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
        // In a real library, you might log this error.
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

YINI_API int yini_get_string(YiniHandle* handle, const char* section, const char* key, char* out_buffer, int buffer_size)
{
    const YiniValue* yini_value = find_value(handle, section, key);
    if (!yini_value || !std::holds_alternative<std::string>(yini_value->value))
    {
        return -1;
    }

    const std::string& str = std::get<std::string>(yini_value->value);

    // Check if buffer is valid
    if (!out_buffer || buffer_size <= 0) {
        return -1;
    }

    // Safely copy the string
    int bytes_to_copy = std::min((int)str.length(), buffer_size - 1);
    memcpy(out_buffer, str.c_str(), bytes_to_copy);
    out_buffer[bytes_to_copy] = '\0';

    return bytes_to_copy;
}

YINI_API int yini_get_int64(YiniHandle* handle, const char* section, const char* key, int64_t* out_value)
{
    const YiniValue* yini_value = find_value(handle, section, key);
    if (!yini_value || !std::holds_alternative<int64_t>(yini_value->value) || !out_value)
    {
        return 0;
    }
    *out_value = std::get<int64_t>(yini_value->value);
    return 1;
}

YINI_API int yini_get_double(YiniHandle* handle, const char* section, const char* key, double* out_value)
{
    const YiniValue* yini_value = find_value(handle, section, key);
    if (!yini_value || !out_value)
    {
        return 0;
    }

    // Allow int to be read as double
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
    if (!yini_value || !std::holds_alternative<bool>(yini_value->value) || !out_value)
    {
        return 0;
    }
    *out_value = std::get<bool>(yini_value->value);
    return 1;
}

} // extern "C"
