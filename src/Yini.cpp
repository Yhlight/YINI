#include "YINI/Yini.h"
#include "YINI/Parser.hpp"
#include "YINI/YiniException.hpp"
#include "YINI/YiniData.hpp"
#include <algorithm>
#include <cstring> // For strncpy
#include <memory>

// Opaque struct definitions - implementation detail of this file
struct YiniDocumentHandle { YINI::YiniDocument doc; };
// The YiniSectionHandle and YiniValueHandle are just reinterpret_casts of the C++ types,
// so they don't need their own struct definitions here.

namespace YINI
{
    // Helper to safely copy strings to a buffer
    static int safe_strncpy(char* dest, const std::string& src, int buffer_size) {
        if (!dest || buffer_size <= 0) return src.length() + 1;
        strncpy(dest, src.c_str(), buffer_size - 1);
        dest[buffer_size - 1] = '\0';
        return src.length() + 1;
    }

    // Helper to find or create a key-value pair
    static YiniKeyValuePair* find_or_create_pair(YiniSection* section, const char* key) {
        if (!section || !key) return nullptr;
        auto it = std::find_if(section->pairs.begin(), section->pairs.end(),
            [&](const YiniKeyValuePair& p) { return p.key == key; });
        if (it != section->pairs.end()) {
            return &(*it);
        } else {
            section->pairs.push_back({std::string(key), {}});
            return &section->pairs.back();
        }
    }

    // Helper to create a complex type and return its handle
    template<typename T>
    YiniValueHandle* create_complex_value(YiniDocumentHandle* handle, const char* section_name, const char* key)
    {
        if (!handle || !section_name || !key) return nullptr;
        YiniSection* section = handle->doc.getOrCreateSection(section_name);
        auto* pair = find_or_create_pair(section, key);
        if (!pair) return nullptr;
        pair->value.data = std::make_unique<T>();
        return reinterpret_cast<YiniValueHandle*>(&pair->value);
    }

    // Template helper for adding primitive values to a container
    template<typename Container, typename T>
    void add_primitive_to_container(YiniValueHandle* value_handle, T value)
    {
        if (!value_handle) return;
        auto* yini_value = reinterpret_cast<YiniValue*>(value_handle);
        if (auto* container_ptr_ptr = std::get_if<std::unique_ptr<Container>>(&yini_value->data))
        {
            if (*container_ptr_ptr)
            {
                YiniValue new_val;
                new_val.data = value;
                if constexpr (std::is_same_v<Container, YiniSet>)
                {
                    (*container_ptr_ptr)->elements.insert(std::move(new_val));
                }
                else
                {
                    (*container_ptr_ptr)->elements.push_back(std::move(new_val));
                }
            }
        }
    }

    // Template helper for setting values in a map
    template<typename T>
    void set_primitive_in_map(YiniValueHandle* value_handle, const char* key, T value)
    {
        if (!value_handle || !key) return;
        auto* yini_value = reinterpret_cast<YiniValue*>(value_handle);
        if (auto* map_ptr_ptr = std::get_if<std::unique_ptr<YiniMap>>(&yini_value->data))
        {
            if (*map_ptr_ptr)
            {
                YiniValue new_val;
                new_val.data = value;
                (*map_ptr_ptr)->elements[std::string(key)] = std::move(new_val);
            }
        }
    }
} // namespace YINI

extern "C" {

// Document API
YINI_API YiniDocumentHandle* yini_parse(const char* content, char* error_buffer, int buffer_size)
{
    if (!content) return nullptr;
    auto* handle = new YiniDocumentHandle();
    try {
        YINI::Parser parser(content, handle->doc, ".");
        parser.parse();
    } catch (const YINI::YiniParsingException& e) {
        if (error_buffer && buffer_size > 0) {
            std::string all_errors;
            for(const auto& err : e.getErrors()) {
                all_errors += "Error at [" + std::to_string(err.line) + ":" + std::to_string(err.column) + "]: " + err.message + "\n";
            }
            YINI::safe_strncpy(error_buffer, all_errors, buffer_size);
        }
        delete handle;
        return nullptr;
    }
    return handle;
}

YINI_API void yini_free_document(YiniDocumentHandle* handle) { delete handle; }

YINI_API int yini_get_section_count(const YiniDocumentHandle* handle) {
    if (!handle) return 0;
    return handle->doc.getSections().size();
}

YINI_API const YiniSectionHandle* yini_get_section_by_index(const YiniDocumentHandle* handle, int index) {
    if (!handle || index < 0 || index >= handle->doc.getSections().size()) return nullptr;
    return reinterpret_cast<const YiniSectionHandle*>(&handle->doc.getSections()[index]);
}

YINI_API const YiniSectionHandle* yini_get_section_by_name(const YiniDocumentHandle* handle, const char* name) {
    if (!handle || !name) return nullptr;
    return reinterpret_cast<const YiniSectionHandle*>(handle->doc.findSection(name));
}

YINI_API void yini_set_string_value(YiniDocumentHandle* handle, const char* section_name, const char* key, const char* value) {
    if (!handle || !section_name || !key || !value) return;
    auto* pair = YINI::find_or_create_pair(handle->doc.getOrCreateSection(section_name), key);
    if(pair) pair->value.data = std::string(value);
}

YINI_API void yini_set_int_value(YiniDocumentHandle* handle, const char* section_name, const char* key, int value) {
    if (!handle || !section_name || !key) return;
    auto* pair = YINI::find_or_create_pair(handle->doc.getOrCreateSection(section_name), key);
    if(pair) pair->value.data = value;
}

YINI_API void yini_set_double_value(YiniDocumentHandle* handle, const char* section_name, const char* key, double value) {
    if (!handle || !section_name || !key) return;
    auto* pair = YINI::find_or_create_pair(handle->doc.getOrCreateSection(section_name), key);
    if(pair) pair->value.data = value;
}

YINI_API void yini_set_bool_value(YiniDocumentHandle* handle, const char* section_name, const char* key, bool value) {
    if (!handle || !section_name || !key) return;
    auto* pair = YINI::find_or_create_pair(handle->doc.getOrCreateSection(section_name), key);
    if(pair) pair->value.data = value;
}

YINI_API YiniValueHandle* yini_create_array_value(YiniDocumentHandle* handle, const char* section, const char* key) {
    return YINI::create_complex_value<YINI::YiniArray>(handle, section, key);
}

YINI_API YiniValueHandle* yini_create_list_value(YiniDocumentHandle* handle, const char* section, const char* key) {
    return YINI::create_complex_value<YINI::YiniList>(handle, section, key);
}

YINI_API YiniValueHandle* yini_create_set_value(YiniDocumentHandle* handle, const char* section, const char* key) {
    return YINI::create_complex_value<YINI::YiniSet>(handle, section, key);
}

YINI_API YiniValueHandle* yini_create_map_value(YiniDocumentHandle* handle, const char* section, const char* key) {
    return YINI::create_complex_value<YINI::YiniMap>(handle, section, key);
}

// Section API
YINI_API int yini_section_get_name(const YiniSectionHandle* section_handle, char* buffer, int buffer_size) {
    if (!section_handle) return 0;
    auto* section = reinterpret_cast<const YINI::YiniSection*>(section_handle);
    return YINI::safe_strncpy(buffer, section->name, buffer_size);
}

YINI_API int yini_section_get_pair_count(const YiniSectionHandle* section_handle) {
    if (!section_handle) return 0;
    return reinterpret_cast<const YINI::YiniSection*>(section_handle)->pairs.size();
}

YINI_API int yini_section_get_pair_key_by_index(const YiniSectionHandle* section_handle, int index, char* buffer, int buffer_size) {
    if (!section_handle) return 0;
    auto* section = reinterpret_cast<const YINI::YiniSection*>(section_handle);
    if (index < 0 || index >= section->pairs.size()) return 0;
    return YINI::safe_strncpy(buffer, section->pairs[index].key, buffer_size);
}

YINI_API const YiniValueHandle* yini_section_get_value_by_key(const YiniSectionHandle* section_handle, const char* key) {
    if (!section_handle || !key) return nullptr;
    auto* section = reinterpret_cast<const YINI::YiniSection*>(section_handle);
    auto it = std::find_if(section->pairs.begin(), section->pairs.end(),
        [&](const YINI::YiniKeyValuePair& p) { return p.key == key; });
    return (it != section->pairs.end()) ? reinterpret_cast<const YiniValueHandle*>(&it->value) : nullptr;
}

// Value API
YINI_API YiniType yini_value_get_type(const YiniValueHandle* value_handle) {
    if (!value_handle) return YINI_TYPE_NONE;
    auto* value = reinterpret_cast<const YINI::YiniValue*>(value_handle);
    if (std::holds_alternative<std::string>(value->data)) return YINI_TYPE_STRING;
    if (std::holds_alternative<int>(value->data)) return YINI_TYPE_INT;
    if (std::holds_alternative<double>(value->data)) return YINI_TYPE_DOUBLE;
    if (std::holds_alternative<bool>(value->data)) return YINI_TYPE_BOOL;
    if (std::holds_alternative<std::unique_ptr<YINI::YiniArray>>(value->data)) return YINI_TYPE_ARRAY;
    if (std::holds_alternative<std::unique_ptr<YINI::YiniList>>(value->data)) return YINI_TYPE_LIST;
    if (std::holds_alternative<std::unique_ptr<YINI::YiniSet>>(value->data)) return YINI_TYPE_SET;
    if (std::holds_alternative<std::unique_ptr<YINI::YiniTuple>>(value->data)) return YINI_TYPE_TUPLE;
    if (std::holds_alternative<std::unique_ptr<YINI::YiniMap>>(value->data)) return YINI_TYPE_MAP;
    if (std::holds_alternative<std::unique_ptr<YINI::YiniDynaValue>>(value->data)) return YINI_TYPE_DYNA;
    if (std::holds_alternative<std::unique_ptr<YINI::YiniCoord>>(value->data)) return YINI_TYPE_COORD;
    if (std::holds_alternative<std::unique_ptr<YINI::YiniColor>>(value->data)) return YINI_TYPE_COLOR;
    if (std::holds_alternative<std::unique_ptr<YINI::YiniPath>>(value->data)) return YINI_TYPE_PATH;
    return YINI_TYPE_NONE;
}

YINI_API int yini_value_get_string(const YiniValueHandle* value_handle, char* buffer, int buffer_size) {
    if (!value_handle) return 0;
    auto* value = reinterpret_cast<const YINI::YiniValue*>(value_handle);
    if (auto* val = std::get_if<std::string>(&value->data)) return YINI::safe_strncpy(buffer, *val, buffer_size);
    return 0;
}

YINI_API int yini_value_get_int(const YiniValueHandle* value_handle) {
    if (!value_handle) return 0;
    auto* value = reinterpret_cast<const YINI::YiniValue*>(value_handle);
    if (auto* val = std::get_if<int>(&value->data)) return *val;
    return 0;
}

YINI_API double yini_value_get_double(const YiniValueHandle* value_handle) {
    if (!value_handle) return 0.0;
    auto* value = reinterpret_cast<const YINI::YiniValue*>(value_handle);
    if (auto* val = std::get_if<double>(&value->data)) return *val;
    return 0.0;
}

YINI_API bool yini_value_get_bool(const YiniValueHandle* value_handle) {
    if (!value_handle) return false;
    auto* value = reinterpret_cast<const YINI::YiniValue*>(value_handle);
    if (auto* val = std::get_if<bool>(&value->data)) return *val;
    return false;
}

// ... implementation for other getters ...
YINI_API void yini_value_get_coord(const YiniValueHandle* value_handle, double* x, double* y, double* z, bool* is_3d) {}
YINI_API void yini_value_get_color(const YiniValueHandle* value_handle, unsigned char* r, unsigned char* g, unsigned char* b) {}
YINI_API int yini_value_get_path(const YiniValueHandle* value_handle, char* buffer, int buffer_size) { return 0; }

// Array API
YINI_API int yini_array_get_size(const YiniValueHandle* value_handle) {
    if (!value_handle) return 0;
    auto* value = reinterpret_cast<const YINI::YiniValue*>(value_handle);
    if (auto* arr_ptr_ptr = std::get_if<std::unique_ptr<YINI::YiniArray>>(&value->data)) {
        return (*arr_ptr_ptr) ? (*arr_ptr_ptr)->elements.size() : 0;
    }
    return 0;
}

YINI_API const YiniValueHandle* yini_array_get_value_by_index(const YiniValueHandle* value_handle, int index) {
    if (!value_handle) return nullptr;
    auto* value = reinterpret_cast<const YINI::YiniValue*>(value_handle);
    if (auto* arr_ptr_ptr = std::get_if<std::unique_ptr<YINI::YiniArray>>(&value->data)) {
        if (*arr_ptr_ptr && index >= 0 && index < (*arr_ptr_ptr)->elements.size()) {
            return reinterpret_cast<const YiniValueHandle*>(&(*arr_ptr_ptr)->elements[index]);
        }
    }
    return nullptr;
}

// List API
YINI_API int yini_list_get_size(const YiniValueHandle* value_handle) {
    if (!value_handle) return 0;
    auto* value = reinterpret_cast<const YINI::YiniValue*>(value_handle);
    if (auto* list_ptr_ptr = std::get_if<std::unique_ptr<YINI::YiniList>>(&value->data)) {
        return (*list_ptr_ptr) ? (*list_ptr_ptr)->elements.size() : 0;
    }
    return 0;
}

YINI_API const YiniValueHandle* yini_list_get_value_by_index(const YiniValueHandle* value_handle, int index) {
    if (!value_handle) return nullptr;
    auto* value = reinterpret_cast<const YINI::YiniValue*>(value_handle);
    if (auto* list_ptr_ptr = std::get_if<std::unique_ptr<YINI::YiniList>>(&value->data)) {
        if (*list_ptr_ptr && index >= 0 && index < (*list_ptr_ptr)->elements.size()) {
            return reinterpret_cast<const YiniValueHandle*>(&(*list_ptr_ptr)->elements[index]);
        }
    }
    return nullptr;
}

// Set API
YINI_API int yini_set_get_size(const YiniValueHandle* value_handle) {
    if (!value_handle) return 0;
    auto* value = reinterpret_cast<const YINI::YiniValue*>(value_handle);
    if (auto* set_ptr_ptr = std::get_if<std::unique_ptr<YINI::YiniSet>>(&value->data)) {
        return (*set_ptr_ptr) ? (*set_ptr_ptr)->elements.size() : 0;
    }
    return 0;
}

YINI_API const YiniValueHandle* yini_set_get_value_by_index(const YiniValueHandle* value_handle, int index) {
    if (!value_handle) return nullptr;
    auto* value = reinterpret_cast<const YINI::YiniValue*>(value_handle);
    if (auto* set_ptr_ptr = std::get_if<std::unique_ptr<YINI::YiniSet>>(&value->data)) {
        if (*set_ptr_ptr && index >= 0 && index < (*set_ptr_ptr)->elements.size()) {
            auto it = std::next((*set_ptr_ptr)->elements.begin(), index);
            return reinterpret_cast<const YiniValueHandle*>(&(*it));
        }
    }
    return nullptr;
}

// Array modification
YINI_API void yini_array_add_string(YiniValueHandle* value_handle, const char* value) { YINI::add_primitive_to_container<YINI::YiniArray>(value_handle, std::string(value)); }
YINI_API void yini_array_add_int(YiniValueHandle* value_handle, int value) { YINI::add_primitive_to_container<YINI::YiniArray>(value_handle, value); }
YINI_API void yini_array_add_double(YiniValueHandle* value_handle, double value) { YINI::add_primitive_to_container<YINI::YiniArray>(value_handle, value); }
YINI_API void yini_array_add_bool(YiniValueHandle* value_handle, bool value) { YINI::add_primitive_to_container<YINI::YiniArray>(value_handle, value); }

// List modification
YINI_API void yini_list_add_string(YiniValueHandle* value_handle, const char* value) { YINI::add_primitive_to_container<YINI::YiniList>(value_handle, std::string(value)); }
YINI_API void yini_list_add_int(YiniValueHandle* value_handle, int value) { YINI::add_primitive_to_container<YINI::YiniList>(value_handle, value); }
YINI_API void yini_list_add_double(YiniValueHandle* value_handle, double value) { YINI::add_primitive_to_container<YINI::YiniList>(value_handle, value); }
YINI_API void yini_list_add_bool(YiniValueHandle* value_handle, bool value) { YINI::add_primitive_to_container<YINI::YiniList>(value_handle, value); }

// Set modification
YINI_API void yini_set_add_string(YiniValueHandle* value_handle, const char* value) { YINI::add_primitive_to_container<YINI::YiniSet>(value_handle, std::string(value)); }
YINI_API void yini_set_add_int(YiniValueHandle* value_handle, int value) { YINI::add_primitive_to_container<YINI::YiniSet>(value_handle, value); }
YINI_API void yini_set_add_double(YiniValueHandle* value_handle, double value) { YINI::add_primitive_to_container<YINI::YiniSet>(value_handle, value); }
YINI_API void yini_set_add_bool(YiniValueHandle* value_handle, bool value) { YINI::add_primitive_to_container<YINI::YiniSet>(value_handle, value); }

// Map API
YINI_API int yini_map_get_size(const YiniValueHandle* value_handle) {
    if (!value_handle) return 0;
    auto* value = reinterpret_cast<const YINI::YiniValue*>(value_handle);
    if (auto* map_ptr_ptr = std::get_if<std::unique_ptr<YINI::YiniMap>>(&value->data)) {
        return (*map_ptr_ptr) ? (*map_ptr_ptr)->elements.size() : 0;
    }
    return 0;
}

YINI_API const YiniValueHandle* yini_map_get_value_by_key(const YiniValueHandle* value_handle, const char* key) {
    if (!value_handle || !key) return nullptr;
    auto* value = reinterpret_cast<const YINI::YiniValue*>(value_handle);
    if (auto* map_ptr_ptr = std::get_if<std::unique_ptr<YINI::YiniMap>>(&value->data)) {
        if (*map_ptr_ptr) {
            auto it = (*map_ptr_ptr)->elements.find(key);
            if (it != (*map_ptr_ptr)->elements.end()) {
                return reinterpret_cast<const YiniValueHandle*>(&it->second);
            }
        }
    }
    return nullptr;
}

YINI_API void yini_map_set_string(YiniValueHandle* value_handle, const char* key, const char* value) { YINI::set_primitive_in_map(value_handle, key, std::string(value)); }
YINI_API void yini_map_set_int(YiniValueHandle* value_handle, const char* key, int value) { YINI::set_primitive_in_map(value_handle, key, value); }
YINI_API void yini_map_set_double(YiniValueHandle* value_handle, const char* key, double value) { YINI::set_primitive_in_map(value_handle, key, value); }
YINI_API void yini_map_set_bool(YiniValueHandle* value_handle, const char* key, bool value) { YINI::set_primitive_in_map(value_handle, key, value); }

} // extern "C"