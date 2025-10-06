#include "YiniCApi.h"
#include "Core/YiniManager.h"
#include "Core/YiniValue.h"
#include "Core/DynaValue.h"
#include "Core/Validator.h"
#include <string>
#include <cstring>
#include <variant>
#include <vector>
#include <map>

// --- Helper Functions to cast handles to internal types ---
static YINI::YiniManager* as_manager(Yini_ManagerHandle handle) {
    return reinterpret_cast<YINI::YiniManager*>(handle);
}

static YINI::YiniValue* as_value(Yini_ValueHandle handle) {
    return reinterpret_cast<YINI::YiniValue*>(handle);
}

// --- Helper for safe string copying ---
static int safe_string_copy(char* out_buffer, int buffer_size, const std::string& str) {
    size_t required_size = str.length() + 1;
    if (out_buffer == nullptr) {
        return static_cast<int>(required_size);
    }
    if (buffer_size < 0 || static_cast<size_t>(buffer_size) < required_size) {
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

// --- Manager Functions ---
YINI_API Yini_ManagerHandle yini_manager_create() {
    return reinterpret_cast<Yini_ManagerHandle>(new YINI::YiniManager());
}

YINI_API void yini_manager_destroy(Yini_ManagerHandle manager) {
    delete as_manager(manager);
}

YINI_API bool yini_manager_load(Yini_ManagerHandle manager, const char* filepath) {
    if (!manager || !filepath) return false;
    try {
        as_manager(manager)->load(filepath);
        return true;
    } catch (...) {
        return false;
    }
}

YINI_API void yini_manager_save_changes(Yini_ManagerHandle manager) {
    if (!manager) return;
    try {
        as_manager(manager)->save_changes();
    } catch (...) {
        // In a real application, you might want to log this error.
    }
}

YINI_API Yini_ValueHandle yini_manager_get_value(Yini_ManagerHandle manager, const char* section, const char* key) {
    if (!manager || !section || !key) return nullptr;
    try {
        YINI::YiniValue value = as_manager(manager)->get_value(section, key);

        // If the retrieved value is dynamic, unwrap it to get the underlying value.
        if (auto* dyna_ptr = std::get_if<std::unique_ptr<YINI::DynaValue>>(&value.m_value)) {
            // Return a new copy of the *inner* value. Caller takes ownership.
            return reinterpret_cast<Yini_ValueHandle>(new YINI::YiniValue((*dyna_ptr)->get()));
        }

        // It's not a dynamic value, so return a new copy of the original.
        return reinterpret_cast<Yini_ValueHandle>(new YINI::YiniValue(value));
    } catch (...) {
        return nullptr;
    }
}

YINI_API void yini_manager_set_value(Yini_ManagerHandle manager, const char* section, const char* key, Yini_ValueHandle value_handle) {
    if (!manager || !section || !key || !value_handle) return;
    try {
        // The value is copied into the manager.
        as_manager(manager)->set_value(section, key, *as_value(value_handle));
    } catch (...) {
        // Handle or log error
    }
}

// --- Schema and Validation Functions ---
YINI_API bool yini_manager_validate(Yini_ManagerHandle manager) {
    if (!manager) return false;
    try {
        return as_manager(manager)->validate();
    } catch (...) {
        return false;
    }
}

YINI_API int yini_manager_get_validation_error_count(Yini_ManagerHandle manager) {
    if (!manager) return 0;
    return static_cast<int>(as_manager(manager)->m_last_validation_errors.size());
}

YINI_API int yini_manager_get_validation_error(Yini_ManagerHandle manager, int index, char* out_buffer, int buffer_size) {
    if (!manager || index < 0) return -1;
    auto* mgr = as_manager(manager);

    if (static_cast<size_t>(index) >= mgr->m_last_validation_errors.size()) {
        return -1; // Index out of bounds.
    }

    const std::string& error_message = mgr->m_last_validation_errors[index].message;
    return safe_string_copy(out_buffer, buffer_size, error_message);
}

// --- Iteration Functions ---
YINI_API int yini_manager_get_section_count(Yini_ManagerHandle manager) {
    if (!manager) return 0;
    return static_cast<int>(as_manager(manager)->get_interpreter().resolved_sections.size());
}

YINI_API int yini_manager_get_section_name_at(Yini_ManagerHandle manager, int index, char* out_buffer, int buffer_size) {
    if (!manager || index < 0) return -1;
    const auto& sections = as_manager(manager)->get_interpreter().resolved_sections;
    if (static_cast<size_t>(index) >= sections.size()) {
        return -1;
    }
    auto it = sections.begin();
    std::advance(it, index);
    return safe_string_copy(out_buffer, buffer_size, it->first);
}

YINI_API int yini_manager_get_key_count_in_section(Yini_ManagerHandle manager, const char* section_name) {
    if (!manager || !section_name) return -1;
    const auto& sections = as_manager(manager)->get_interpreter().resolved_sections;
    auto it = sections.find(section_name);
    if (it == sections.end()) {
        return -1; // Section not found
    }
    return static_cast<int>(it->second.size());
}

YINI_API int yini_manager_get_key_name_at(Yini_ManagerHandle manager, const char* section_name, int index, char* out_buffer, int buffer_size) {
    if (!manager || !section_name || index < 0) return -1;
    const auto& sections = as_manager(manager)->get_interpreter().resolved_sections;
    auto sec_it = sections.find(section_name);
    if (sec_it == sections.end()) {
        return -1; // Section not found
    }
    const auto& keys = sec_it->second;
    if (static_cast<size_t>(index) >= keys.size()) {
        return -1; // Key index out of bounds
    }
    auto key_it = keys.begin();
    std::advance(key_it, index);
    return safe_string_copy(out_buffer, buffer_size, key_it->first);
}

// --- Value Handle Functions ---
YINI_API void yini_value_destroy(Yini_ValueHandle handle) {
    delete as_value(handle);
}

YINI_API YiniValueType yini_value_get_type(Yini_ValueHandle handle) {
    if (!handle) return YiniValueType_Null;
    YINI::YiniValue* value = as_value(handle);
    if (std::holds_alternative<std::monostate>(value->m_value)) return YiniValueType_Null;
    if (std::holds_alternative<bool>(value->m_value)) return YiniValueType_Bool;
    if (std::holds_alternative<double>(value->m_value)) return YiniValueType_Double;
    if (std::holds_alternative<std::string>(value->m_value)) return YiniValueType_String;
    if (std::holds_alternative<std::unique_ptr<YINI::YiniArray>>(value->m_value)) return YiniValueType_Array;
    if (std::holds_alternative<std::unique_ptr<YINI::YiniMap>>(value->m_value)) return YiniValueType_Map;
    if (std::holds_alternative<std::unique_ptr<YINI::DynaValue>>(value->m_value)) return YiniValueType_Dyna;
    return YiniValueType_Null;
}

// --- Create Value Handles ---
YINI_API Yini_ValueHandle yini_value_create_double(double value) { return reinterpret_cast<Yini_ValueHandle>(new YINI::YiniValue(value)); }
YINI_API Yini_ValueHandle yini_value_create_string(const char* value) { return reinterpret_cast<Yini_ValueHandle>(new YINI::YiniValue(std::string(value ? value : ""))); }
YINI_API Yini_ValueHandle yini_value_create_bool(bool value) { return reinterpret_cast<Yini_ValueHandle>(new YINI::YiniValue(value)); }
YINI_API Yini_ValueHandle yini_value_create_array() { return reinterpret_cast<Yini_ValueHandle>(new YINI::YiniValue(YINI::YiniArray{})); }
YINI_API Yini_ValueHandle yini_value_create_map() { return reinterpret_cast<Yini_ValueHandle>(new YINI::YiniValue(YINI::YiniMap{})); }


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
        return safe_string_copy(out_buffer, buffer_size, *str_ptr);
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
        // Return a new copy of the inner value. Caller takes ownership.
        return reinterpret_cast<Yini_ValueHandle>(new YINI::YiniValue((*val)->get()));
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
            // Return a new copy of the element. Caller takes ownership.
            return reinterpret_cast<Yini_ValueHandle>(new YINI::YiniValue((*arr_ptr)->at(index)));
        }
    }
    return nullptr;
}

YINI_API void yini_array_add_element(Yini_ValueHandle array_handle, Yini_ValueHandle element_handle) {
    if (!array_handle || !element_handle) return;
    if (auto* arr_ptr = std::get_if<std::unique_ptr<YINI::YiniArray>>(&as_value(array_handle)->m_value)) {
        // The element is copied into the array.
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
            // Return a new copy of the value. Caller takes ownership.
            return reinterpret_cast<Yini_ValueHandle>(new YINI::YiniValue(it->second));
        }
    }
    return nullptr;
}

YINI_API int yini_map_get_key_at(Yini_ValueHandle handle, int index, char* out_buffer, int buffer_size) {
    if (!handle || index < 0) return -1;
    if (const auto* map_ptr = std::get_if<std::unique_ptr<YINI::YiniMap>>(&as_value(handle)->m_value)) {
        if (static_cast<size_t>(index) < (*map_ptr)->size()) {
            auto it = (*map_ptr)->begin();
            std::advance(it, index);
            return safe_string_copy(out_buffer, buffer_size, it->first);
        }
    }
    return -1;
}

YINI_API void yini_map_set_value(Yini_ValueHandle map_handle, const char* key, Yini_ValueHandle value_handle) {
    if (!map_handle || !key || !value_handle) return;
    if (auto* map_ptr = std::get_if<std::unique_ptr<YINI::YiniMap>>(&as_value(map_handle)->m_value)) {
        // The value is copied into the map.
        (**map_ptr)[key] = *as_value(value_handle);
    }
}