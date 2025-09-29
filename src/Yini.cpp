#include "YINI/Yini.h"
#include "YINI/Parser.hpp"
#include "YINI/YiniException.hpp"
#include <algorithm>
#include <cstring> // For strncpy

// Opaque struct for the document handle
struct YiniDocumentHandle { YINI::YiniDocument doc; };

// Helper to safely copy strings to a buffer
static int safe_strncpy(char* dest, const std::string& src, int buffer_size) {
    if (!dest || buffer_size <= 0) return src.length() + 1;
    strncpy(dest, src.c_str(), buffer_size - 1);
    dest[buffer_size - 1] = '\0';
    return src.length() + 1; // Return the required buffer size
}

// Helper to find or create a key-value pair
static YINI::YiniKeyValuePair* find_or_create_pair(YINI::YiniSection* section, const char* key) {
    if (!section || !key) return nullptr;

    auto it = std::find_if(section->pairs.begin(), section->pairs.end(),
        [&](const YINI::YiniKeyValuePair& p) { return p.key == key; });

    if (it != section->pairs.end()) {
        return &(*it);
    } else {
        section->pairs.push_back({std::string(key), {}});
        return &section->pairs.back();
    }
}

// C-style wrapper functions
extern "C" {

// Document API
YINI_API YiniDocumentHandle* yini_parse(const char* content, char* error_buffer, int buffer_size)
{
    if (!content) return nullptr;
    auto* handle = new YiniDocumentHandle();
    try {
        YINI::Parser parser(content, handle->doc, ".");
        parser.parse();
        handle->doc.resolveInheritance();
    } catch (const YINI::YiniException& e) {
        if (error_buffer && buffer_size > 0) {
            std::string error_msg = "Error at [" + std::to_string(e.getLine()) + ":" + std::to_string(e.getColumn()) + "]: " + e.what();
            safe_strncpy(error_buffer, error_msg, buffer_size);
        }
        delete handle;
        return nullptr;
    }
    return handle;
}

YINI_API void yini_free_document(YiniDocumentHandle* handle)
{
    delete handle;
}

YINI_API int yini_get_section_count(const YiniDocumentHandle* handle)
{
    if (!handle) return 0;
    return handle->doc.getSections().size();
}

YINI_API const YiniSectionHandle* yini_get_section_by_index(const YiniDocumentHandle* handle, int index)
{
    if (!handle || index < 0 || index >= handle->doc.getSections().size()) return nullptr;
    return reinterpret_cast<const YiniSectionHandle*>(&handle->doc.getSections()[index]);
}

YINI_API const YiniSectionHandle* yini_get_section_by_name(const YiniDocumentHandle* handle, const char* name)
{
    if (!handle || !name) return nullptr;
    return reinterpret_cast<const YiniSectionHandle*>(handle->doc.findSection(name));
}

YINI_API void yini_set_string_value(YiniDocumentHandle* handle, const char* section_name, const char* key, const char* value)
{
    if (!handle || !section_name || !key || !value) return;
    YINI::YiniSection* section = handle->doc.getOrCreateSection(section_name);
    auto* pair = find_or_create_pair(section, key);
    if(pair) pair->value.data = std::string(value);
}

YINI_API void yini_set_int_value(YiniDocumentHandle* handle, const char* section_name, const char* key, int value)
{
    if (!handle || !section_name || !key) return;
    YINI::YiniSection* section = handle->doc.getOrCreateSection(section_name);
    auto* pair = find_or_create_pair(section, key);
    if(pair) pair->value.data = value;
}

YINI_API void yini_set_double_value(YiniDocumentHandle* handle, const char* section_name, const char* key, double value)
{
    if (!handle || !section_name || !key) return;
    YINI::YiniSection* section = handle->doc.getOrCreateSection(section_name);
    auto* pair = find_or_create_pair(section, key);
    if(pair) pair->value.data = value;
}

YINI_API void yini_set_bool_value(YiniDocumentHandle* handle, const char* section_name, const char* key, bool value)
{
    if (!handle || !section_name || !key) return;
    YINI::YiniSection* section = handle->doc.getOrCreateSection(section_name);
    auto* pair = find_or_create_pair(section, key);
    if(pair) pair->value.data = value;
}

YINI_API int yini_get_define_count(const YiniDocumentHandle* handle)
{
    if (!handle) return 0;
    return handle->doc.getDefines().size();
}

YINI_API const YiniValueHandle* yini_get_define_by_index(const YiniDocumentHandle* handle, int index, char* key_buffer, int key_buffer_size)
{
    if (!handle || index < 0 || index >= handle->doc.getDefines().size()) {
        return nullptr;
    }
    const auto& defines = handle->doc.getDefines();
    auto it = defines.begin();
    std::advance(it, index);
    safe_strncpy(key_buffer, it->first, key_buffer_size);
    return reinterpret_cast<const YiniValueHandle*>(&it->second);
}


// Section API
YINI_API int yini_section_get_name(const YiniSectionHandle* section_handle, char* buffer, int buffer_size)
{
    if (!section_handle) return 0;
    auto* section = reinterpret_cast<const YINI::YiniSection*>(section_handle);
    return safe_strncpy(buffer, section->name, buffer_size);
}

YINI_API int yini_section_get_pair_count(const YiniSectionHandle* section_handle)
{
    if (!section_handle) return 0;
    auto* section = reinterpret_cast<const YINI::YiniSection*>(section_handle);
    return section->pairs.size();
}

YINI_API int yini_section_get_pair_key_by_index(const YiniSectionHandle* section_handle, int index, char* buffer, int buffer_size)
{
    if (!section_handle) return 0;
    auto* section = reinterpret_cast<const YINI::YiniSection*>(section_handle);
    if (index < 0 || index >= section->pairs.size()) return 0;
    return safe_strncpy(buffer, section->pairs[index].key, buffer_size);
}

YINI_API const YiniValueHandle* yini_section_get_value_by_key(const YiniSectionHandle* section_handle, const char* key)
{
    if (!section_handle || !key) return nullptr;
    auto* section = reinterpret_cast<const YINI::YiniSection*>(section_handle);
    auto it = std::find_if(section->pairs.begin(), section->pairs.end(),
        [&](const YINI::YiniKeyValuePair& p) { return p.key == key; });
    if (it != section->pairs.end()) {
        return reinterpret_cast<const YiniValueHandle*>(&it->value);
    }
    return nullptr;
}

YINI_API int yini_section_get_registration_count(const YiniSectionHandle* section_handle)
{
    if (!section_handle) return 0;
    auto* section = reinterpret_cast<const YINI::YiniSection*>(section_handle);
    return section->registrationList.size();
}

YINI_API const YiniValueHandle* yini_section_get_registered_value_by_index(const YiniSectionHandle* section_handle, int index)
{
    if (!section_handle) return nullptr;
    auto* section = reinterpret_cast<const YINI::YiniSection*>(section_handle);
    if (index < 0 || index >= section->registrationList.size()) return nullptr;
    return reinterpret_cast<const YiniValueHandle*>(&section->registrationList[index]);
}


// Value API
YINI_API YiniType yini_value_get_type(const YiniValueHandle* value_handle)
{
    if (!value_handle) return YINI_TYPE_NONE;
    auto* value = reinterpret_cast<const YINI::YiniValue*>(value_handle);
    if (std::holds_alternative<std::string>(value->data)) return YINI_TYPE_STRING;
    if (std::holds_alternative<int>(value->data)) return YINI_TYPE_INT;
    if (std::holds_alternative<double>(value->data)) return YINI_TYPE_DOUBLE;
    if (std::holds_alternative<bool>(value->data)) return YINI_TYPE_BOOL;
    if (std::holds_alternative<std::unique_ptr<YINI::YiniArray>>(value->data)) return YINI_TYPE_ARRAY;
    if (std::holds_alternative<std::unique_ptr<YINI::YiniList>>(value->data)) return YINI_TYPE_LIST;
    if (std::holds_alternative<std::unique_ptr<YINI::YiniSet>>(value->data)) return YINI_TYPE_SET;
    if (std::holds_alternative<std::unique_ptr<YINI::YiniMap>>(value->data)) return YINI_TYPE_MAP;
    if (std::holds_alternative<std::unique_ptr<YINI::YiniDynaValue>>(value->data)) return YINI_TYPE_DYNA;
    if (std::holds_alternative<std::unique_ptr<YINI::YiniCoord>>(value->data)) return YINI_TYPE_COORD;
    if (std::holds_alternative<std::unique_ptr<YINI::YiniColor>>(value->data)) return YINI_TYPE_COLOR;
    if (std::holds_alternative<std::unique_ptr<YINI::YiniPath>>(value->data)) return YINI_TYPE_PATH;
    return YINI_TYPE_NONE;
}

YINI_API int yini_value_get_string(const YiniValueHandle* value_handle, char* buffer, int buffer_size)
{
    if (!value_handle) return 0;
    auto* value = reinterpret_cast<const YINI::YiniValue*>(value_handle);
    if (!std::holds_alternative<std::string>(value->data)) return 0;
    return safe_strncpy(buffer, std::get<std::string>(value->data), buffer_size);
}

YINI_API int yini_value_get_int(const YiniValueHandle* value_handle)
{
    if (!value_handle) return 0;
    auto* value = reinterpret_cast<const YINI::YiniValue*>(value_handle);
    if (!std::holds_alternative<int>(value->data)) return 0;
    return std::get<int>(value->data);
}

YINI_API double yini_value_get_double(const YiniValueHandle* value_handle)
{
    if (!value_handle) return 0.0;
    auto* value = reinterpret_cast<const YINI::YiniValue*>(value_handle);
    if (!std::holds_alternative<double>(value->data)) return 0.0;
    return std::get<double>(value->data);
}

YINI_API bool yini_value_get_bool(const YiniValueHandle* value_handle)
{
    if (!value_handle) return false;
    auto* value = reinterpret_cast<const YINI::YiniValue*>(value_handle);
    if (!std::holds_alternative<bool>(value->data)) return false;
    return std::get<bool>(value->data);
}

YINI_API void yini_value_get_coord(const YiniValueHandle* value_handle, double* x, double* y, double* z, bool* is_3d)
{
    if (!value_handle || !x || !y || !z || !is_3d) return;
    auto* value = reinterpret_cast<const YINI::YiniValue*>(value_handle);
    if (!std::holds_alternative<std::unique_ptr<YINI::YiniCoord>>(value->data)) return;
    const auto& coord = std::get<std::unique_ptr<YINI::YiniCoord>>(value->data);
    *x = coord->x;
    *y = coord->y;
    *z = coord->z;
    *is_3d = coord->is_3d;
}

YINI_API void yini_value_get_color(const YiniValueHandle* value_handle, unsigned char* r, unsigned char* g, unsigned char* b)
{
    if (!value_handle || !r || !g || !b) return;
    auto* value = reinterpret_cast<const YINI::YiniValue*>(value_handle);
    if (!std::holds_alternative<std::unique_ptr<YINI::YiniColor>>(value->data)) return;
    const auto& color = std::get<std::unique_ptr<YINI::YiniColor>>(value->data);
    *r = color->r;
    *g = color->g;
    *b = color->b;
}

YINI_API int yini_value_get_path(const YiniValueHandle* value_handle, char* buffer, int buffer_size)
{
    if (!value_handle) return 0;
    auto* value = reinterpret_cast<const YINI::YiniValue*>(value_handle);
    if (!std::holds_alternative<std::unique_ptr<YINI::YiniPath>>(value->data)) return 0;
    const auto& path = std::get<std::unique_ptr<YINI::YiniPath>>(value->data);
    return safe_strncpy(buffer, path->path_value, buffer_size);
}

YINI_API int yini_array_get_size(const YiniValueHandle* value_handle)
{
    if (!value_handle) return 0;
    auto* value = reinterpret_cast<const YINI::YiniValue*>(value_handle);
    if (!std::holds_alternative<std::unique_ptr<YINI::YiniArray>>(value->data)) return 0;

    const auto& arr_ptr = std::get<std::unique_ptr<YINI::YiniArray>>(value->data);
    if (!arr_ptr) return 0;

    return arr_ptr->elements.size();
}

YINI_API const YiniValueHandle* yini_array_get_value_by_index(const YiniValueHandle* value_handle, int index)
{
    if (!value_handle) return nullptr;
    auto* value = reinterpret_cast<const YINI::YiniValue*>(value_handle);
    if (!std::holds_alternative<std::unique_ptr<YINI::YiniArray>>(value->data)) return nullptr;

    const auto& arr_ptr = std::get<std::unique_ptr<YINI::YiniArray>>(value->data);
    if (!arr_ptr || index < 0 || index >= arr_ptr->elements.size()) return nullptr;

    return reinterpret_cast<const YiniValueHandle*>(&arr_ptr->elements[index]);
}

// List API
YINI_API int yini_list_get_size(const YiniValueHandle* value_handle)
{
    if (!value_handle) return 0;
    auto* value = reinterpret_cast<const YINI::YiniValue*>(value_handle);
    if (!std::holds_alternative<std::unique_ptr<YINI::YiniList>>(value->data)) return 0;
    const auto& list_ptr = std::get<std::unique_ptr<YINI::YiniList>>(value->data);
    if (!list_ptr) return 0;
    return list_ptr->elements.size();
}

YINI_API const YiniValueHandle* yini_list_get_value_by_index(const YiniValueHandle* value_handle, int index)
{
    if (!value_handle) return nullptr;
    auto* value = reinterpret_cast<const YINI::YiniValue*>(value_handle);
    if (!std::holds_alternative<std::unique_ptr<YINI::YiniList>>(value->data)) return nullptr;
    const auto& list_ptr = std::get<std::unique_ptr<YINI::YiniList>>(value->data);
    if (!list_ptr || index < 0 || index >= list_ptr->elements.size()) return nullptr;
    return reinterpret_cast<const YiniValueHandle*>(&list_ptr->elements[index]);
}

// Set API
YINI_API int yini_set_get_size(const YiniValueHandle* value_handle)
{
    if (!value_handle) return 0;
    auto* value = reinterpret_cast<const YINI::YiniValue*>(value_handle);
    if (!std::holds_alternative<std::unique_ptr<YINI::YiniSet>>(value->data)) return 0;
    const auto& set_ptr = std::get<std::unique_ptr<YINI::YiniSet>>(value->data);
    if (!set_ptr) return 0;
    return set_ptr->elements.size();
}

YINI_API const YiniValueHandle* yini_set_get_value_by_index(const YiniValueHandle* value_handle, int index)
{
    if (!value_handle) return nullptr;
    auto* value = reinterpret_cast<const YINI::YiniValue*>(value_handle);
    if (!std::holds_alternative<std::unique_ptr<YINI::YiniSet>>(value->data)) return nullptr;
    const auto& set_ptr = std::get<std::unique_ptr<YINI::YiniSet>>(value->data);
    if (!set_ptr || index < 0 || index >= set_ptr->elements.size()) return nullptr;
    return reinterpret_cast<const YiniValueHandle*>(&set_ptr->elements[index]);
}

// Map API
YINI_API int yini_map_get_size(const YiniValueHandle* value_handle)
{
    if (!value_handle) return 0;
    auto* value = reinterpret_cast<const YINI::YiniValue*>(value_handle);
    if (!std::holds_alternative<std::unique_ptr<YINI::YiniMap>>(value->data)) return 0;
    const auto& map_ptr = std::get<std::unique_ptr<YINI::YiniMap>>(value->data);
    if (!map_ptr) return 0;
    return map_ptr->elements.size();
}

YINI_API int yini_map_get_key_by_index(const YiniValueHandle* value_handle, int index, char* buffer, int buffer_size)
{
    if (!value_handle) return 0;
    auto* value = reinterpret_cast<const YINI::YiniValue*>(value_handle);
    if (!std::holds_alternative<std::unique_ptr<YINI::YiniMap>>(value->data)) return 0;

    const auto& map_ptr = std::get<std::unique_ptr<YINI::YiniMap>>(value->data);
    if (!map_ptr || index < 0 || index >= map_ptr->elements.size()) return 0;

    auto it = map_ptr->elements.begin();
    std::advance(it, index);
    return safe_strncpy(buffer, it->first, buffer_size);
}

YINI_API const YiniValueHandle* yini_map_get_value_by_key(const YiniValueHandle* value_handle, const char* key)
{
    if (!value_handle || !key) return nullptr;
    auto* value = reinterpret_cast<const YINI::YiniValue*>(value_handle);
    if (!std::holds_alternative<std::unique_ptr<YINI::YiniMap>>(value->data)) return nullptr;

    const auto& map_ptr = std::get<std::unique_ptr<YINI::YiniMap>>(value->data);
    if (!map_ptr) return nullptr;

    auto it = map_ptr->elements.find(key);
    if (it != map_ptr->elements.end()) {
        return reinterpret_cast<const YiniValueHandle*>(&it->second);
    }

    return nullptr;
}

} // extern "C"