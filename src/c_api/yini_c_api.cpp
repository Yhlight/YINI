#include "yini_c_api.h"
#include "../Parser/Parser.h"
#include "../Resolver/Resolver.h"
#include <fstream>
#include <sstream>
#include <map>
#include <string>
#include <cstring>

// A simple struct to hold the parsed YINI data in memory
struct YiniHandle {
    std::unique_ptr<YINI::AstNode> ast;
};

extern "C" {

YINI_C_API void* yini_load(const char* filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return nullptr;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    try {
        YINI::Lexer lexer(content);
        YINI::Parser parser(lexer, filepath);
        auto ast = parser.parse();

        YINI::Resolver resolver;
        resolver.resolve(*ast);

        YiniHandle* handle = new YiniHandle{std::move(ast)};
        return handle;
    } catch (...) {
        return nullptr;
    }
}

YINI_C_API void yini_free(void* handle) {
    if (handle) {
        delete static_cast<YiniHandle*>(handle);
    }
}

// Helper to find a value. For simplicity, this is not very efficient.
YINI::YiniValue* find_value(YiniHandle* handle, const char* section_name, const char* key_name) {
    if (!handle || !handle->ast) return nullptr;

    for (auto& section : handle->ast->sections) {
        if (section.name == section_name) {
            for (auto& kv : section.key_values) {
                if (kv.key == key_name) {
                    return kv.value.get();
                }
            }
        }
    }
    return nullptr;
}

YINI_C_API const char* yini_get_string(void* handle, const char* section, const char* key) {
    YiniHandle* h = static_cast<YiniHandle*>(handle);
    YINI::YiniValue* val = find_value(h, section, key);
    if (val && std::holds_alternative<std::string>(val->value)) {
        const std::string& s = std::get<std::string>(val->value);
        char* cstr = new char[s.length() + 1];
        std::strcpy(cstr, s.c_str());
        return cstr;
    }
    return nullptr;
}

YINI_C_API void yini_free_string(const char* str) {
    if (str) {
        delete[] str;
    }
}

YINI_C_API int yini_get_int(void* handle, const char* section, const char* key) {
    YiniHandle* h = static_cast<YiniHandle*>(handle);
    YINI::YiniValue* val = find_value(h, section, key);
    if (val && std::holds_alternative<int64_t>(val->value)) {
        return static_cast<int>(std::get<int64_t>(val->value));
    }
    return 0; // Default value
}

YINI_C_API double yini_get_double(void* handle, const char* section, const char* key) {
    YiniHandle* h = static_cast<YiniHandle*>(handle);
    YINI::YiniValue* val = find_value(h, section, key);
    if (val && std::holds_alternative<double>(val->value)) {
        return std::get<double>(val->value);
    }
    return 0.0; // Default value
}

YINI_C_API bool yini_get_bool(void* handle, const char* section, const char* key) {
    YiniHandle* h = static_cast<YiniHandle*>(handle);
    YINI::YiniValue* val = find_value(h, section, key);
    if (val && std::holds_alternative<bool>(val->value)) {
        return std::get<bool>(val->value);
    }
    return false; // Default value
}

}