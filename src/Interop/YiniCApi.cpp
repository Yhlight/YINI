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
    if (!handle) return nullptr;
    return reinterpret_cast<YINI::YiniManager*>(handle);
}

static YINI::YiniValue* as_value(Yini_ValueHandle handle) {
    if (!handle) return nullptr;
    return reinterpret_cast<YINI::YiniValue*>(handle);
}

// --- Helper to set the last error on the manager ---
static void set_last_error(Yini_ManagerHandle handle, const std::string& message) {
    if (auto* mgr = as_manager(handle)) {
        mgr->m_last_error = message;
    }
}

static void set_last_error(Yini_ManagerHandle handle, const std::exception& e) {
    if (auto* mgr = as_manager(handle)) {
        mgr->m_last_error = e.what();
    }
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
    auto* mgr = as_manager(manager);
    if (!mgr) {
        return false;
    }
    mgr->m_last_error.clear();
    if (!filepath) {
        set_last_error(manager, "Filepath cannot be null.");
        return false;
    }
    try {
        mgr->load(std::string_view(filepath));
        return true;
    } catch (const std::exception& e) {
        set_last_error(manager, e);
        return false;
    } catch (...) {
        set_last_error(manager, "An unknown error occurred during file loading.");
        return false;
    }
}

YINI_API bool yini_manager_load_from_string(Yini_ManagerHandle manager, const char* content, const char* virtual_filepath) {
    auto* mgr = as_manager(manager);
    if (!mgr) {
        return false;
    }
    mgr->m_last_error.clear();
    if (!content || !virtual_filepath) {
        set_last_error(manager, "Content and virtual_filepath cannot be null.");
        return false;
    }
    try {
        mgr->load_from_string(std::string_view(content), std::string_view(virtual_filepath));
        return true;
    } catch (const std::exception& e) {
        set_last_error(manager, e);
        return false;
    } catch (...) {
        set_last_error(manager, "An unknown error occurred during string loading.");
        return false;
    }
}

YINI_API void yini_manager_save_changes(Yini_ManagerHandle manager) {
    auto* mgr = as_manager(manager);
    if (!mgr) return;
    mgr->m_last_error.clear();
    try {
        mgr->save_changes();
    } catch (const std::exception& e) {
        set_last_error(manager, e);
    } catch (...) {
        set_last_error(manager, "An unknown error occurred while saving changes.");
    }
}

YINI_API Yini_ValueHandle yini_manager_get_value(Yini_ManagerHandle manager, const char* section, const char* key) {
    auto* mgr = as_manager(manager);
    if (!mgr) return nullptr;
    mgr->m_last_error.clear();

    if (!section || !key) {
        set_last_error(manager, "Section and key cannot be null.");
        return nullptr;
    }
    try {
        YINI::YiniValue value = mgr->get_value(std::string_view(section), std::string_view(key));
        return reinterpret_cast<Yini_ValueHandle>(new YINI::YiniValue(std::move(value)));
    } catch (const std::exception& e) {
        set_last_error(manager, e);
        return nullptr;
    } catch (...) {
        set_last_error(manager, "An unknown error occurred while getting value.");
        return nullptr;
    }
}

YINI_API bool yini_manager_has_key(Yini_ManagerHandle manager, const char* section, const char* key) {
    auto* mgr = as_manager(manager);
    if (!mgr || !section || !key) {
        return false;
    }
    const auto& interpreter = mgr->get_interpreter();
    auto sec_it = interpreter.resolved_sections.find(std::string_view(section));
    if (sec_it != interpreter.resolved_sections.end()) {
        return sec_it->second.find(std::string_view(key)) != sec_it->second.end();
    }
    return false;
}

YINI_API int yini_manager_get_last_error(Yini_ManagerHandle manager, char* out_buffer, int buffer_size) {
    auto* mgr = as_manager(manager);
    if (!mgr || mgr->m_last_error.empty()) {
        return 0;
    }

    int result = safe_string_copy(out_buffer, buffer_size, mgr->m_last_error);
    if (out_buffer != nullptr && buffer_size > 0) {
        mgr->m_last_error.clear();
    }
    return result;
}

YINI_API int yini_manager_get_macro_count(Yini_ManagerHandle manager) {
    auto* mgr = as_manager(manager);
    if (!mgr) return 0;
    return static_cast<int>(mgr->get_interpreter().get_macro_names().size());
}

YINI_API int yini_manager_get_macro_name_at(Yini_ManagerHandle manager, int index, char* out_buffer, int buffer_size) {
    auto* mgr = as_manager(manager);
    if (!mgr || index < 0) return -1;

    const auto macro_names = mgr->get_interpreter().get_macro_names();
    if (static_cast<size_t>(index) >= macro_names.size()) {
        return -1;
    }

    return safe_string_copy(out_buffer, buffer_size, macro_names[index]);
}


YINI_API void yini_manager_set_value(Yini_ManagerHandle manager, const char* section, const char* key, Yini_ValueHandle value_handle) {
    auto* mgr = as_manager(manager);
    if (!mgr) return;
    mgr->m_last_error.clear();

    if (!section || !key || !value_handle) {
        set_last_error(manager, "Section, key, and value handle cannot be null.");
        return;
    }
    try {
        mgr->set_value(std::string_view(section), std::string_view(key), *as_value(value_handle));
    } catch (const std::exception& e) {
        set_last_error(manager, e);
    } catch (...) {
        set_last_error(manager, "An unknown error occurred while setting value.");
    }
}

YINI_API int yini_manager_find_key_at_pos(Yini_ManagerHandle manager, int line, int column, char* out_section, int* section_size, char* out_key, int* key_size)
{
    auto* mgr = as_manager(manager);
    if (!mgr) return 0;

    const auto& kv_map = mgr->get_interpreter().get_kv_map();
    for (const auto& section_pair : kv_map)
    {
        for (const auto& key_pair : section_pair.second)
        {
            const auto* kv_node = key_pair.second;
            const auto& key_token = kv_node->key;

            if (key_token.line == line &&
                column >= key_token.column &&
                column < (key_token.column + static_cast<int>(key_token.lexeme.length())))
            {
                const std::string& section_name = section_pair.first;
                const std::string& key_name = key_pair.first;

                *section_size = static_cast<int>(section_name.length() + 1);
                *key_size = static_cast<int>(key_name.length() + 1);

                if (out_section != nullptr && out_key != nullptr)
                {
                    safe_string_copy(out_section, *section_size, section_name);
                    safe_string_copy(out_key, *key_size, key_name);
                }
                return 1; // Found
            }
        }
    }
    return 0; // Not found
}

YINI_API int yini_manager_validate(Yini_ManagerHandle manager) {
    if (!manager) return -1;
    auto* mgr = as_manager(manager);
    mgr->m_last_validation_errors.clear();

    const YINI::Schema* schema = mgr->get_schema();
    if (!schema) {
        return 0;
    }

    try {
        YINI::Validator validator;
        mgr->m_last_validation_errors = validator.validate(*schema, mgr->get_interpreter());
        return static_cast<int>(mgr->m_last_validation_errors.size());
    } catch (...) {
        return -1;
    }
}

YINI_API bool yini_manager_get_definition_location(Yini_ManagerHandle manager, const char* section_name, const char* symbol_name, char* out_filepath, int* filepath_size, int* out_line, int* out_column)
{
    auto* mgr = as_manager(manager);
    if (!mgr || !symbol_name || !filepath_size || !out_line || !out_column)
    {
        return false;
    }

    const auto& interpreter = mgr->get_interpreter();
    std::optional<YINI::Token> token;

    if (section_name != nullptr)
    {
        // It's a key in a section
        const auto& kv_map = interpreter.get_kv_map();
        auto section_it = kv_map.find(section_name);
        if (section_it != kv_map.end())
        {
            auto key_it = section_it->second.find(symbol_name);
            if (key_it != section_it->second.end())
            {
                token = key_it->second->key;
            }
        }
    }
    else
    {
        // It's a macro
        token = interpreter.get_macro_definition_token(std::string_view(symbol_name));
    }

    if (token.has_value())
    {
        *filepath_size = static_cast<int>(token->filepath.length() + 1);
        *out_line = token->line;
        *out_column = token->column;

        if (out_filepath != nullptr)
        {
            safe_string_copy(out_filepath, *filepath_size, token->filepath);
        }
        return true;
    }

    return false;
}

YINI_API int yini_manager_get_validation_error(Yini_ManagerHandle manager, int index, char* out_buffer, int buffer_size) {
    if (!manager || index < 0) return -1;
    auto* mgr = as_manager(manager);

    if (static_cast<size_t>(index) >= mgr->m_last_validation_errors.size()) {
        return -1;
    }

    const std::string& error_message = mgr->m_last_validation_errors[index].message;
    return safe_string_copy(out_buffer, buffer_size, error_message);
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
            return reinterpret_cast<Yini_ValueHandle>(new YINI::YiniValue((*arr_ptr)->at(index)));
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
        if ((*map_ptr)->size() > static_cast<size_t>(index)) {
            auto it = (*map_ptr)->begin();
            std::advance(it, index);
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
        (**map_ptr)[key] = *as_value(value_handle);
    }
}