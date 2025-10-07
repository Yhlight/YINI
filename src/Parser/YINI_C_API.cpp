#include "YINI_C_API.h"
#include "Parser.h"
#include "Interpreter.h"
#include "Section.h"
#include "YMETA.h"
#include <vector>
#include <string>
#include <iostream>
#include <cstring>
#include <fstream>
#include <sstream>

// This global variable will store the last error message for inspection
// when a handle creation fails (returns NULL).
static std::string g_last_error;

// --- Helper Functions ---

static yini::Interpreter* to_interpreter(YiniParserHandle handle)
{
    return static_cast<yini::Interpreter*>(handle);
}

static const yini::Section* to_section(YiniSectionHandle handle)
{
    return static_cast<const yini::Section*>(handle);
}

static const yini::Value* to_value(YiniValueHandle handle)
{
    return static_cast<const yini::Value*>(handle);
}

static char* c_string_from_std(const std::string& s)
{
    char* cstr = new char[s.length() + 1];
    std::strcpy(cstr, s.c_str());
    return cstr;
}

static YiniValueType to_c_value_type(yini::ValueType cpp_type)
{
    switch (cpp_type)
    {
        case yini::ValueType::INTEGER: return YINI_TYPE_INTEGER;
        case yini::ValueType::FLOAT:   return YINI_TYPE_FLOAT;
        case yini::ValueType::BOOLEAN: return YINI_TYPE_BOOLEAN;
        case yini::ValueType::STRING:  return YINI_TYPE_STRING;
        case yini::ValueType::ARRAY:   return YINI_TYPE_ARRAY;
        case yini::ValueType::LIST:    return YINI_TYPE_ARRAY;
        case yini::ValueType::MAP:     return YINI_TYPE_MAP;
        case yini::ValueType::COLOR:   return YINI_TYPE_COLOR;
        case yini::ValueType::COORD:   return YINI_TYPE_COORD;
        default:                       return YINI_TYPE_NIL;
    }
}

// --- Parser Lifecycle & Error Handling ---

YINI_API YiniParserHandle yini_parser_create(const char* source)
{
    if (!source)
    {
        g_last_error = "Input source was null.";
        return nullptr;
    }

    yini::Parser parser(source);
    auto ast = parser.parse();
    if (parser.hasError())
    {
        g_last_error = parser.getLastError();
        return nullptr;
    }

    auto interpreter = new yini::Interpreter();
    if (!interpreter->interpret(*ast))
    {
        g_last_error = interpreter->getLastError();
        delete interpreter;
        return nullptr;
    }

    g_last_error = "";
    return interpreter;
}

YINI_API YiniParserHandle yini_parser_create_from_file(const char* filename)
{
    if (!filename)
    {
        g_last_error = "Input filename was null.";
        return nullptr;
    }
    std::ifstream file(filename);
    if (!file)
    {
        g_last_error = "Failed to open file: " + std::string(filename);
        return nullptr;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return yini_parser_create(buffer.str().c_str());
}

YINI_API void yini_parser_destroy(YiniParserHandle parser)
{
    if (parser)
    {
        delete to_interpreter(parser);
    }
}

YINI_API const char* yini_parser_get_error(YiniParserHandle parser)
{
    if (parser)
    {
        // This function is for handles that were created successfully.
        // For creation errors, use the static g_last_error.
        return c_string_from_std(to_interpreter(parser)->getLastError());
    }
    return c_string_from_std(g_last_error);
}

// --- Section Access ---

YINI_API int yini_parser_get_section_count(YiniParserHandle parser)
{
    if (!parser) return 0;
    return to_interpreter(parser)->getSections().size();
}

YINI_API const char** yini_parser_get_section_names(YiniParserHandle parser, int* count)
{
    if (!parser || !count) return nullptr;
    const auto& sections = to_interpreter(parser)->getSections();
    *count = sections.size();
    if (*count == 0) return nullptr;

    const char** names = new const char*[*count];
    int i = 0;
    for (const auto& pair : sections)
    {
        names[i++] = c_string_from_std(pair.first);
    }
    return names;
}

YINI_API YiniSectionHandle yini_parser_get_section(YiniParserHandle parser, const char* name)
{
    if (!parser || !name) return nullptr;
    const auto& sections = to_interpreter(parser)->getSections();
    auto it = sections.find(name);
    if (it != sections.end())
    {
        return const_cast<yini::Section*>(&it->second);
    }
    return nullptr;
}

YINI_API int yini_section_get_key_count(YiniSectionHandle section)
{
    if (!section) return 0;
    return to_section(section)->entries.size();
}

YINI_API const char** yini_section_get_keys(YiniSectionHandle section, int* count)
{
    if (!section || !count) return nullptr;
    const auto& entries = to_section(section)->entries;
    *count = entries.size();
    if (*count == 0) return nullptr;

    const char** keys = new const char*[*count];
    int i = 0;
    for (const auto& pair : entries)
    {
        keys[i++] = c_string_from_std(pair.first);
    }
    return keys;
}

// --- Value Access ---

YINI_API YiniValueHandle yini_section_get_value(YiniSectionHandle section, const char* key)
{
    if (!section || !key) return nullptr;
    const auto& entries = to_section(section)->entries;
    auto it = entries.find(key);
    if (it != entries.end())
    {
        return it->second.get();
    }
    return nullptr;
}

YINI_API YiniValueType yini_value_get_type(YiniValueHandle value)
{
    if (!value) return YINI_TYPE_NIL;
    return to_c_value_type(to_value(value)->getType());
}

YINI_API int64_t yini_value_get_integer(YiniValueHandle value)
{
    if (!value || !to_value(value)->isInteger()) return 0;
    return to_value(value)->asInteger();
}

YINI_API double yini_value_get_float(YiniValueHandle value)
{
    if (!value || !to_value(value)->isNumeric()) return 0.0;
    return to_value(value)->asFloat();
}

YINI_API bool yini_value_get_boolean(YiniValueHandle value)
{
    if (!value || !to_value(value)->isBoolean()) return false;
    return to_value(value)->asBoolean();
}

YINI_API const char* yini_value_get_string(YiniValueHandle value)
{
    if (!value || !to_value(value)->isString()) return c_string_from_std("");
    return c_string_from_std(to_value(value)->asString());
}

// --- Array Access ---

YINI_API int yini_value_get_array_size(YiniValueHandle value)
{
    if (!value || !to_value(value)->isArray()) return 0;
    return to_value(value)->asArray().size();
}

YINI_API YiniValueHandle yini_value_get_array_element(YiniValueHandle value, int index)
{
    if (!value || !to_value(value)->isArray()) return nullptr;
    const auto& arr = to_value(value)->asArray();
    if (index < 0 || static_cast<size_t>(index) >= arr.size()) return nullptr;
    return arr[index].get();
}

// --- Memory Management ---

YINI_API void yini_free_string(const char* str)
{
    delete[] str;
}

YINI_API void yini_free_string_array(const char** array, int count)
{
    if (!array) return;
    for (int i = 0; i < count; ++i)
    {
        delete[] array[i];
    }
    delete[] array;
}

// --- YMETA Utility Functions ---

YINI_API bool yini_compile_to_ymeta(const char* input_file, const char* output_file)
{
    if (!input_file || !output_file) return false;
    
    YiniParserHandle handle = yini_parser_create_from_file(input_file);
    if (!handle) return false;

    yini::YMETA ymeta;
    bool result = ymeta.serialize(*to_interpreter(handle), output_file);
    yini_parser_destroy(handle);
    return result;
}

YINI_API bool yini_decompile_from_ymeta(const char* input_file, const char* output_file)
{
    if (!input_file || !output_file) return false;

    yini::YMETA ymeta;
    if (!ymeta.deserialize(input_file)) return false;

    std::ofstream out(output_file);
    if (!out) return false;
    
    out << ymeta.toYINI();
    return true;
}