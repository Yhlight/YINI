#include "YINI/Yini.h"
#include "YINI/Parser.hpp"
#include "YINI/YiniManager.hpp"
#include "YINI/YiniException.hpp"
#include "YiniValueToString.hpp"
#include <algorithm>
#include <cstring> // For strncpy

// Opaque struct definitions
struct YiniManagerHandle { YINI::YiniManager manager; };
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

//==============================================================================
// Manager API
//==============================================================================

YINI_API YiniManagerHandle* yini_manager_create(const char* yini_file_path)
{
    if (!yini_file_path) return nullptr;
    try {
        // The YiniManager constructor can throw or fail to load.
        // We wrap this in a try-catch and check is_loaded().
        auto* handle = new YiniManagerHandle{YINI::YiniManager(yini_file_path)};
        if (!handle->manager.is_loaded()) {
            delete handle;
            return nullptr;
        }
        return handle;
    } catch (...) {
        // Catch any other potential exceptions during construction
        return nullptr;
    }
}

YINI_API void yini_manager_free(YiniManagerHandle* handle)
{
    delete handle; // Destructor of YiniManagerHandle will call ~YiniManager
}

YINI_API bool yini_manager_is_loaded(const YiniManagerHandle* handle)
{
    if (!handle) return false;
    return handle->manager.is_loaded();
}

YINI_API const YiniDocumentHandle* yini_manager_get_document(YiniManagerHandle* handle)
{
    if (!handle) return nullptr;
    // The document is a member of the manager, so we can safely cast its address.
    // The lifetime of this document handle is tied to the manager handle.
    // The caller MUST NOT free this document handle directly.
    return reinterpret_cast<const YiniDocumentHandle*>(&handle->manager.get_document());
}

YINI_API void yini_manager_set_string_value(YiniManagerHandle* handle, const char* section, const char* key, const char* value)
{
    if (!handle || !section || !key || !value) return;
    handle->manager.set_string_value(section, key, value);
}

YINI_API void yini_manager_set_int_value(YiniManagerHandle* handle, const char* section, const char* key, int value)
{
    if (!handle || !section || !key) return;
    handle->manager.set_int_value(section, key, value);
}

YINI_API void yini_manager_set_double_value(YiniManagerHandle* handle, const char* section, const char* key, double value)
{
    if (!handle || !section || !key) return;
    handle->manager.set_double_value(section, key, value);
}

YINI_API void yini_manager_set_bool_value(YiniManagerHandle* handle, const char* section, const char* key, bool value)
{
    if (!handle || !section || !key) return;
    handle->manager.set_bool_value(section, key, value);
}

//==============================================================================
// Document API
//==============================================================================

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
    const auto* doc = reinterpret_cast<const YINI::YiniDocument*>(handle);
    return doc->getSections().size();
}

YINI_API const YiniSectionHandle* yini_get_section_by_index(const YiniDocumentHandle* handle, int index)
{
    if (!handle) return nullptr;
    const auto* doc = reinterpret_cast<const YINI::YiniDocument*>(handle);
    if (index < 0 || index >= doc->getSections().size()) return nullptr;
    return reinterpret_cast<const YiniSectionHandle*>(&doc->getSections()[index]);
}

YINI_API const YiniSectionHandle* yini_get_section_by_name(const YiniDocumentHandle* handle, const char* name)
{
    if (!handle || !name) return nullptr;
    const auto* doc = reinterpret_cast<const YINI::YiniDocument*>(handle);
    return reinterpret_cast<const YiniSectionHandle*>(doc->findSection(name));
}

YINI_API void yini_set_string_value(YiniDocumentHandle* handle, const char* section_name, const char* key, const char* value)
{
    if (!handle || !section_name || !key || !value) return;
    auto* doc = reinterpret_cast<YINI::YiniDocument*>(handle);
    YINI::YiniSection* section = doc->getOrCreateSection(section_name);
    auto* pair = find_or_create_pair(section, key);
    if(pair) pair->value.data = std::string(value);
}

YINI_API void yini_set_int_value(YiniDocumentHandle* handle, const char* section_name, const char* key, int value)
{
    if (!handle || !section_name || !key) return;
    auto* doc = reinterpret_cast<YINI::YiniDocument*>(handle);
    YINI::YiniSection* section = doc->getOrCreateSection(section_name);
    auto* pair = find_or_create_pair(section, key);
    if(pair) pair->value.data = value;
}

YINI_API void yini_set_double_value(YiniDocumentHandle* handle, const char* section_name, const char* key, double value)
{
    if (!handle || !section_name || !key) return;
    auto* doc = reinterpret_cast<YINI::YiniDocument*>(handle);
    YINI::YiniSection* section = doc->getOrCreateSection(section_name);
    auto* pair = find_or_create_pair(section, key);
    if(pair) pair->value.data = value;
}

YINI_API void yini_set_bool_value(YiniDocumentHandle* handle, const char* section_name, const char* key, bool value)
{
    if (!handle || !section_name || !key) return;
    auto* doc = reinterpret_cast<YINI::YiniDocument*>(handle);
    YINI::YiniSection* section = doc->getOrCreateSection(section_name);
    auto* pair = find_or_create_pair(section, key);
    if(pair) pair->value.data = value;
}

YINI_API int yini_get_define_count(const YiniDocumentHandle* handle)
{
    if (!handle) return 0;
    const auto* doc = reinterpret_cast<const YINI::YiniDocument*>(handle);
    return doc->getDefines().size();
}

YINI_API const YiniValueHandle* yini_get_define_by_index(const YiniDocumentHandle* handle, int index, char* key_buffer, int key_buffer_size)
{
    if (!handle || index < 0) return nullptr;
    const auto* doc = reinterpret_cast<const YINI::YiniDocument*>(handle);
    if (index >= doc->getDefines().size()) return nullptr;

    const auto& defines = doc->getDefines();
    auto it = defines.begin();
    std::advance(it, index);
    safe_strncpy(key_buffer, it->first, key_buffer_size);
    return reinterpret_cast<const YiniValueHandle*>(&it->second);
}

YINI_API const YiniValueHandle* yini_get_define_by_key(const YiniDocumentHandle* handle, const char* key)
{
    if (!handle || !key) return nullptr;
    const auto* doc = reinterpret_cast<const YINI::YiniDocument*>(handle);
    const auto& defines = doc->getDefines();
    auto it = defines.find(key);
    if (it != defines.end()) {
        return reinterpret_cast<const YiniValueHandle*>(&it->second);
    }
    return nullptr;
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

YINI_API bool yini_value_get_int(const YiniValueHandle* value_handle, int* out_value)
{
    if (!value_handle || !out_value) return false;
    auto* value = reinterpret_cast<const YINI::YiniValue*>(value_handle);
    if (!std::holds_alternative<int>(value->data)) return false;
    *out_value = std::get<int>(value->data);
    return true;
}

YINI_API bool yini_value_get_double(const YiniValueHandle* value_handle, double* out_value)
{
    if (!value_handle || !out_value) return false;
    auto* value = reinterpret_cast<const YINI::YiniValue*>(value_handle);
    if (!std::holds_alternative<double>(value->data)) return false;
    *out_value = std::get<double>(value->data);
    return true;
}

YINI_API bool yini_value_get_bool(const YiniValueHandle* value_handle, bool* out_value)
{
    if (!value_handle || !out_value) return false;
    auto* value = reinterpret_cast<const YINI::YiniValue*>(value_handle);
    if (!std::holds_alternative<bool>(value->data)) return false;
    *out_value = std::get<bool>(value->data);
    return true;
}

YINI_API bool yini_value_get_coord(const YiniValueHandle* value_handle, double* x, double* y, double* z, bool* is_3d)
{
    if (!value_handle || !x || !y || !z || !is_3d) return false;
    auto* value = reinterpret_cast<const YINI::YiniValue*>(value_handle);
    if (!std::holds_alternative<std::unique_ptr<YINI::YiniCoord>>(value->data)) return false;
    const auto& coord = std::get<std::unique_ptr<YINI::YiniCoord>>(value->data);
    if (!coord) return false;
    *x = coord->x;
    *y = coord->y;
    *z = coord->z;
    *is_3d = coord->is_3d;
    return true;
}

YINI_API bool yini_value_get_color(const YiniValueHandle* value_handle, unsigned char* r, unsigned char* g, unsigned char* b)
{
    if (!value_handle || !r || !g || !b) return false;
    auto* value = reinterpret_cast<const YINI::YiniValue*>(value_handle);
    if (!std::holds_alternative<std::unique_ptr<YINI::YiniColor>>(value->data)) return false;
    const auto& color = std::get<std::unique_ptr<YINI::YiniColor>>(value->data);
    if (!color) return false;
    *r = color->r;
    *g = color->g;
    *b = color->b;
    return true;
}

YINI_API int yini_value_get_path(const YiniValueHandle* value_handle, char* buffer, int buffer_size)
{
    if (!value_handle) return 0;
    auto* value = reinterpret_cast<const YINI::YiniValue*>(value_handle);
    if (!std::holds_alternative<std::unique_ptr<YINI::YiniPath>>(value->data)) return 0;
    const auto& path = std::get<std::unique_ptr<YINI::YiniPath>>(value->data);
    return safe_strncpy(buffer, path->pathValue, buffer_size);
}

YINI_API int yini_value_to_string(const YiniValueHandle* value_handle, char* buffer, int buffer_size)
{
    if (!value_handle) return 0;
    auto* value = reinterpret_cast<const YINI::YiniValue*>(value_handle);
    std::string str_value = YINI::valueToString(*value);
    return safe_strncpy(buffer, str_value, buffer_size);
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