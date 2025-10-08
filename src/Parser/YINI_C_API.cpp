#include "YINI_C_API.h"
#include "Parser.h"
#include "Lexer.h"
#include "YMETA.h"
#include <fstream>
#include <cstring>

using namespace yini;

// Internal helper to convert Parser* to handle
static inline Parser* to_parser(YiniParserHandle handle)
{
    return static_cast<Parser*>(handle);
}

static inline const Section* to_section(YiniSectionHandle handle)
{
    return static_cast<const Section*>(handle);
}

static inline const std::shared_ptr<Value>* to_value(YiniValueHandle handle)
{
    return static_cast<const std::shared_ptr<Value>*>(handle);
}

// Parser functions

YiniParserHandle yini_parser_create(const char* source)
{
    if (!source)
    {
        return nullptr;
    }
    
    try
    {
        Parser* parser = new Parser(std::string(source));
        return static_cast<YiniParserHandle>(parser);
    }
    catch (...)
    {
        return nullptr;
    }
}

YiniParserHandle yini_parser_create_from_file(const char* filename)
{
    if (!filename)
    {
        return nullptr;
    }
    
    try
    {
        std::ifstream file(filename);
        if (!file.is_open())
        {
            return nullptr;
        }
        
        std::string source((std::istreambuf_iterator<char>(file)),
                          std::istreambuf_iterator<char>());
        file.close();
        
        Parser* parser = new Parser(source);
        return static_cast<YiniParserHandle>(parser);
    }
    catch (...)
    {
        return nullptr;
    }
}

void yini_parser_destroy(YiniParserHandle parser)
{
    if (parser)
    {
        delete to_parser(parser);
    }
}

bool yini_parser_parse(YiniParserHandle parser)
{
    if (!parser)
    {
        return false;
    }
    
    try
    {
        return to_parser(parser)->parse();
    }
    catch (...)
    {
        return false;
    }
}

const char* yini_parser_get_error(YiniParserHandle parser)
{
    if (!parser)
    {
        return "Invalid parser handle";
    }
    
    try
    {
        static std::string error;
        error = to_parser(parser)->getLastError();
        return error.c_str();
    }
    catch (...)
    {
        return "Unknown error";
    }
}

// Section functions

int yini_parser_get_section_count(YiniParserHandle parser)
{
    if (!parser)
    {
        return 0;
    }
    
    try
    {
        return static_cast<int>(to_parser(parser)->getSections().size());
    }
    catch (...)
    {
        return 0;
    }
}

const char** yini_parser_get_section_names(YiniParserHandle parser, int* count)
{
    if (!parser || !count)
    {
        if (count) *count = 0;
        return nullptr;
    }
    
    try
    {
        const auto& sections = to_parser(parser)->getSections();
        *count = static_cast<int>(sections.size());
        
        const char** names = new const char*[*count];
        int i = 0;
        for (const auto& [name, section] : sections)
        {
            (void)section; // Unused
            char* name_copy = new char[name.length() + 1];
            std::strcpy(name_copy, name.c_str());
            names[i++] = name_copy;
        }
        
        return names;
    }
    catch (...)
    {
        *count = 0;
        return nullptr;
    }
}

YiniSectionHandle yini_parser_get_section(YiniParserHandle parser, const char* name)
{
    if (!parser || !name)
    {
        return nullptr;
    }
    
    try
    {
        const auto& sections = to_parser(parser)->getSections();
        auto it = sections.find(name);
        if (it == sections.end())
        {
            return nullptr;
        }
        
        return const_cast<Section*>(&it->second);
    }
    catch (...)
    {
        return nullptr;
    }
}

int yini_section_get_entry_count(YiniSectionHandle section)
{
    if (!section)
    {
        return 0;
    }
    
    try
    {
        return static_cast<int>(to_section(section)->entries.size());
    }
    catch (...)
    {
        return 0;
    }
}

const char** yini_section_get_keys(YiniSectionHandle section, int* count)
{
    if (!section || !count)
    {
        if (count) *count = 0;
        return nullptr;
    }
    
    try
    {
        const auto& entries = to_section(section)->entries;
        *count = static_cast<int>(entries.size());
        
        const char** keys = new const char*[*count];
        int i = 0;
        for (const auto& [key, value] : entries)
        {
            (void)value; // Unused
            char* key_copy = new char[key.length() + 1];
            std::strcpy(key_copy, key.c_str());
            keys[i++] = key_copy;
        }
        
        return keys;
    }
    catch (...)
    {
        *count = 0;
        return nullptr;
    }
}

// Value functions

YiniValueHandle yini_section_get_value(YiniSectionHandle section, const char* key)
{
    if (!section || !key)
    {
        return nullptr;
    }
    
    try
    {
        const auto& entries = to_section(section)->entries;
        auto it = entries.find(key);
        if (it == entries.end())
        {
            return nullptr;
        }
        
        return const_cast<std::shared_ptr<Value>*>(&it->second);
    }
    catch (...)
    {
        return nullptr;
    }
}

YiniValueType yini_value_get_type(YiniValueHandle value)
{
    if (!value)
    {
        return YINI_TYPE_NIL;
    }
    
    try
    {
        auto val = *to_value(value);
        switch (val->getType())
        {
            case ValueType::INTEGER: return YINI_TYPE_INTEGER;
            case ValueType::FLOAT: return YINI_TYPE_FLOAT;
            case ValueType::BOOLEAN: return YINI_TYPE_BOOLEAN;
            case ValueType::STRING: return YINI_TYPE_STRING;
            case ValueType::ARRAY: return YINI_TYPE_ARRAY;
            case ValueType::MAP: return YINI_TYPE_MAP;
            case ValueType::COLOR: return YINI_TYPE_COLOR;
            case ValueType::COORD: return YINI_TYPE_COORD;
            default: return YINI_TYPE_NIL;
        }
    }
    catch (...)
    {
        return YINI_TYPE_NIL;
    }
}

int64_t yini_value_get_integer(YiniValueHandle value)
{
    if (!value)
    {
        return 0;
    }
    
    try
    {
        return (*to_value(value))->asInteger();
    }
    catch (...)
    {
        return 0;
    }
}

double yini_value_get_float(YiniValueHandle value)
{
    if (!value)
    {
        return 0.0;
    }
    
    try
    {
        return (*to_value(value))->asFloat();
    }
    catch (...)
    {
        return 0.0;
    }
}

bool yini_value_get_boolean(YiniValueHandle value)
{
    if (!value)
    {
        return false;
    }
    
    try
    {
        return (*to_value(value))->asBoolean();
    }
    catch (...)
    {
        return false;
    }
}

const char* yini_value_get_string(YiniValueHandle value)
{
    if (!value)
    {
        return "";
    }
    
    try
    {
        static std::string str;
        str = (*to_value(value))->asString();
        return str.c_str();
    }
    catch (...)
    {
        return "";
    }
}

int yini_value_get_array_size(YiniValueHandle value)
{
    if (!value)
    {
        return 0;
    }
    
    try
    {
        return static_cast<int>((*to_value(value))->asArray().size());
    }
    catch (...)
    {
        return 0;
    }
}

YiniValueHandle yini_value_get_array_element(YiniValueHandle value, int index)
{
    if (!value || index < 0)
    {
        return nullptr;
    }
    
    try
    {
        // Note: This returns a pointer to a shared_ptr inside the array
        // The caller must not store this pointer long-term
        static thread_local std::shared_ptr<Value> element;
        
        auto arr = (*to_value(value))->asArray();
        if (index >= static_cast<int>(arr.size()))
        {
            return nullptr;
        }
        
        element = arr[index];
        return &element;
    }
    catch (...)
    {
        return nullptr;
    }
}

// Memory management

void yini_free_string_array(const char** array, int count)
{
    if (!array)
    {
        return;
    }
    
    for (int i = 0; i < count; i++)
    {
        delete[] array[i];
    }
    delete[] array;
}

// YMETA functions

bool yini_compile_to_ymeta(const char* input_file, const char* output_file)
{
    if (!input_file || !output_file)
    {
        return false;
    }
    
    try
    {
        std::ifstream file(input_file);
        if (!file.is_open())
        {
            return false;
        }
        
        std::string source((std::istreambuf_iterator<char>(file)),
                          std::istreambuf_iterator<char>());
        file.close();
        
        Parser parser(source);
        if (!parser.parse())
        {
            return false;
        }
        
        YMETA ymeta;
        return ymeta.serialize(parser, output_file);
    }
    catch (...)
    {
        return false;
    }
}

bool yini_decompile_from_ymeta(const char* input_file, const char* output_file)
{
    if (!input_file || !output_file)
    {
        return false;
    }
    
    try
    {
        YMETA ymeta;
        if (!ymeta.deserialize(input_file))
        {
            return false;
        }
        
        std::string yini_text = ymeta.toYINI();
        
        std::ofstream out(output_file);
        if (!out.is_open())
        {
            return false;
        }
        
        out << yini_text;
        out.close();
        
        return true;
    }
    catch (...)
    {
        return false;
    }
}
