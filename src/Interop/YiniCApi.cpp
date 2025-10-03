#include "YiniCApi.h"
#include "Core/YiniManager.h"
#include "Core/YiniValue.h"
#include "Core/DynaValue.h"
#include <string>
#include <cstring>
#include <variant>
#include <vector>
#include <map>

// --- Helper Functions ---
static YINI::YiniManager* as_manager(Yini_ManagerHandle handle) {
    return static_cast<YINI::YiniManager*>(handle);
}

static YINI::YiniValue* as_value(Yini_ValueHandle handle) {
    return static_cast<YINI::YiniValue*>(handle);
}

// Thread-safe storage for string keys returned by the map API
static std::vector<std::string> g_key_buffer;
static const char* store_key(std::string&& key) {
    g_key_buffer.push_back(std::move(key));
    return g_key_buffer.back().c_str();
}

// --- Manager Functions ---
YINI_API Yini_ManagerHandle yini_manager_create() {
    return new YINI::YiniManager();
}

YINI_API void yini_manager_destroy(Yini_ManagerHandle manager) {
    delete as_manager(manager);
}

YINI_API bool yini_manager_load(Yini_ManagerHandle manager, const char* filepath) {
    try {
        as_manager(manager)->load(filepath);
        return true;
    } catch (...) {
        return false;
    }
}

YINI_API void yini_manager_save_changes(Yini_ManagerHandle manager) {
    try {
        as_manager(manager)->save_changes();
    } catch (...) {
        // In a real application, you might want to log this error.
    }
}

// --- Value Get/Set on Manager ---
YINI_API Yini_ValueHandle yini_manager_get_value(Yini_ManagerHandle manager, const char* section, const char* key) {
    try {
        YINI::YiniValue value = as_manager(manager)->get_value(section, key);
        return new YINI::YiniValue(std::move(value));
    } catch (...) {
        return nullptr;
    }
}

YINI_API void yini_manager_set_value(Yini_ManagerHandle manager, const char* section, const char* key, Yini_ValueHandle value_handle) {
    if (!value_handle) return;
    try {
        as_manager(manager)->set_value(section, key, *as_value(value_handle));
    } catch (...) {
        // Handle or log error
    }
}

// --- Value Handle Functions ---
YINI_API void yini_value_destroy(Yini_ValueHandle handle) {
    delete as_value(handle);
}

YINI_API Yini_ValueType yini_value_get_type(Yini_ValueHandle handle) {
    if (!handle) return YINI_TYPE_NULL;
    YINI::YiniValue* value = as_value(handle);
    if (std::holds_alternative<std::monostate>(value->m_value)) return YINI_TYPE_NULL;
    if (std::holds_alternative<bool>(value->m_value)) return YINI_TYPE_BOOL;
    if (std::holds_alternative<double>(value->m_value)) return YINI_TYPE_DOUBLE;
    if (std::holds_alternative<std::string>(value->m_value)) return YINI_TYPE_STRING;
    if (std::holds_alternative<std::unique_ptr<YINI::YiniArray>>(value->m_value)) return YINI_TYPE_ARRAY;
    if (std::holds_alternative<std::unique_ptr<YINI::YiniMap>>(value->m_value)) return YINI_TYPE_MAP;
    if (std::holds_alternative<std::unique_ptr<YINI::DynaValue>>(value->m_value)) return YINI_TYPE_DYNA;
    return YINI_TYPE_NULL;
}

// --- Create Value Handles ---
YINI_API Yini_ValueHandle yini_value_create_double(double value) { return new YINI::YiniValue(value); }
YINI_API Yini_ValueHandle yini_value_create_string(const char* value) { return new YINI::YiniValue(std::string(value)); }
YINI_API Yini_ValueHandle yini_value_create_bool(bool value) { return new YINI::YiniValue(value); }
YINI_API Yini_ValueHandle yini_value_create_array() { return new YINI::YiniValue(YINI::YiniArray{}); }
YINI_API Yini_ValueHandle yini_value_create_map() { return new YINI::YiniValue(YINI::YiniMap{}); }


// --- Get Data from Value Handles ---
YINI_API bool yini_value_get_double(Yini_ValueHandle handle, double* out_value) {
    if (!handle || !out_value) return false;
    if (const auto* val = std::get_if<double>(&as_value(handle)->m_value)) {
        *out_value = *val;
        return true;
    }
    return false;
}

YINI_API int yini_value_get_string(Yini_ValueHandle handle, char* out_buffer, int buffer_size) {
    if (!handle) return -1;
    if (const auto* str_ptr = std::get_if<std::string>(&as_value(handle)->m_value)) {
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
    return -1;
}

YINI_API bool yini_value_get_bool(Yini_ValueHandle handle, bool* out_value) {
    if (!handle || !out_value) return false;
    if (const auto* val = std::get_if<bool>(&as_value(handle)->m_value)) {
        *out_value = *val;
        return true;
    }
    return false;
}

YINI_API Yini_ValueHandle yini_value_get_dyna_value(Yini_ValueHandle handle) {
    if (!handle) return nullptr;
    if (const auto* val = std::get_if<std::unique_ptr<YINI::DynaValue>>(&as_value(handle)->m_value)) {
        return new YINI::YiniValue((*val)->get());
    }
    return nullptr;
}


// --- Array Manipulation ---
YINI_API int yini_array_get_size(Yini_ValueHandle handle) {
    if (!handle) return -1;
    if (const auto* arr_ptr = std::get_if<std::unique_ptr<YINI::YiniArray>>(&as_value(handle)->m_value)) {
        return static_cast<int>((*arr_ptr)->size());
    }
    return -1;
}

YINI_API Yini_ValueHandle yini_array_get_element(Yini_ValueHandle handle, int index) {
    if (!handle || index < 0) return nullptr;
    if (const auto* arr_ptr = std::get_if<std::unique_ptr<YINI::YiniArray>>(&as_value(handle)->m_value)) {
        if (static_cast<size_t>(index) < (*arr_ptr)->size()) {
            return new YINI::YiniValue((*arr_ptr)->at(index));
        }
    }
    return nullptr;
}

YINI_API void yini_array_add_element(Yini_ValueHandle array_handle, Yini_ValueHandle element_handle) {
    if (!array_handle || !element_handle) return;
    if (auto* arr_ptr = std::get_if<std::unique_ptr<YINI::YiniArray>>(&as_value(array_handle)->m_value)) {
        (*arr_ptr)->push_back(*as_value(element_handle));
    }
}

// --- Map Manipulation ---
YINI_API int yini_map_get_size(Yini_ValueHandle handle) {
    if (!handle) return -1;
    if (const auto* map_ptr = std::get_if<std::unique_ptr<YINI::YiniMap>>(&as_value(handle)->m_value)) {
        return static_cast<int>((*map_ptr)->size());
    }
    return -1;
}

YINI_API Yini_ValueHandle yini_map_get_value_at(Yini_ValueHandle handle, int index) {
    if (!handle || index < 0) return nullptr;
    if (const auto* map_ptr = std::get_if<std::unique_ptr<YINI::YiniMap>>(&as_value(handle)->m_value)) {
        if (static_cast<size_t>(index) < (*map_ptr)->size()) {
            auto it = (*map_ptr)->begin();
            std::advance(it, index);
            return new YINI::YiniValue(it->second);
        }
    }
    return nullptr;
}

YINI_API const char* yini_map_get_key_at(Yini_ValueHandle handle, int index) {
    if (!handle || index < 0) return nullptr;
    if (const auto* map_ptr = std::get_if<std::unique_ptr<YINI::YiniMap>>(&as_value(handle)->m_value)) {
        if (static_cast<size_t>(index) < (*map_ptr)->size()) {
            auto it = (*map_ptr)->begin();
            std::advance(it, index);
            return store_key(std::string(it->first));
        }
    }
    return nullptr;
}

YINI_API void yini_map_set_value(Yini_ValueHandle map_handle, const char* key, Yini_ValueHandle value_handle) {
    if (!map_handle || !key || !value_handle) return;
    if (auto* map_ptr = std::get_if<std::unique_ptr<YINI::YiniMap>>(&as_value(map_handle)->m_value)) {
        (**map_ptr)[key] = *as_value(value_handle);
    }
}