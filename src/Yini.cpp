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
    if (std::holds_alternative<std::unique_ptr<YINI::YiniMap>>(value->data)) return YINI_TYPE_MAP;
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
    if (!std::holds_alternative<int>(value->data)) return 0; // Or throw error? For now, return 0.
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

} // extern "C"