#include "YiniCApi.h"
#include "Core/YiniManager.h"
#include "Core/YiniValue.h"
#include <string>
#include <cstring>
#include <variant>

// Helper to safely cast the void pointer back to a YiniManager
static YINI::YiniManager* as_manager(void* manager) {
    return static_cast<YINI::YiniManager*>(manager);
}

YINI_API void* yini_manager_create() {
    return new YINI::YiniManager();
}

YINI_API void yini_manager_destroy(void* manager) {
    delete as_manager(manager);
}

YINI_API bool yini_manager_load(void* manager, const char* filepath) {
    try {
        as_manager(manager)->load(filepath);
        return true;
    } catch (...) {
        return false;
    }
}

YINI_API void yini_manager_save_changes(void* manager) {
    try {
        as_manager(manager)->save_changes();
    } catch (...) {
        // Handle or log error if necessary
    }
}

YINI_API bool yini_manager_get_double(void* manager, const char* section, const char* key, double* out_value) {
    try {
        YINI::YiniValue value = as_manager(manager)->get_value(section, key);
        if (const double* val_ptr = std::get_if<double>(&value.m_value)) {
            *out_value = *val_ptr;
            return true;
        }
    } catch (...) {}
    return false;
}

YINI_API int yini_manager_get_string(void* manager, const char* section, const char* key, char* out_buffer, int buffer_size) {
    try {
        YINI::YiniValue value = as_manager(manager)->get_value(section, key);
        if (const std::string* str_ptr = std::get_if<std::string>(&value.m_value)) {
            const std::string& str = *str_ptr;
            size_t required_size = str.length() + 1;

            if (out_buffer == nullptr || buffer_size < 0 || static_cast<size_t>(buffer_size) < required_size) {
                return static_cast<int>(required_size);
            }

            #ifdef _WIN32
                strcpy_s(out_buffer, buffer_size, str.c_str());
            #else
                strncpy(out_buffer, str.c_str(), buffer_size);
                out_buffer[buffer_size - 1] = '\0';
            #endif
            return static_cast<int>(str.length());
        }
    } catch (...) {
        return -1;
    }
    return -1; // Not found or wrong type
}

YINI_API bool yini_manager_get_bool(void* manager, const char* section, const char* key, bool* out_value) {
    try {
        YINI::YiniValue value = as_manager(manager)->get_value(section, key);
        if (const bool* val_ptr = std::get_if<bool>(&value.m_value)) {
            *out_value = *val_ptr;
            return true;
        }
    } catch (...) {}
    return false;
}

YINI_API void yini_manager_set_double(void* manager, const char* section, const char* key, double value) {
    as_manager(manager)->set_value(section, key, YINI::YiniValue(value));
}

YINI_API void yini_manager_set_string(void* manager, const char* section, const char* key, const char* value) {
    as_manager(manager)->set_value(section, key, YINI::YiniValue(std::string(value)));
}

YINI_API void yini_manager_set_bool(void* manager, const char* section, const char* key, bool value) {
    as_manager(manager)->set_value(section, key, YINI::YiniValue(value));
}