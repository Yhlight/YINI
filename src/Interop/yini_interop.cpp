#include "yini_interop.h"
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "Resolver/Resolver.h"
#include "Validator/Validator.h"
#include "Ymeta/YmetaManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <any>
#include <cstring>

namespace YINI
{
    class Config
    {
    public:
        Config(const std::string& file_path)
        {
            std::ifstream file(file_path);
            if (!file.is_open())
            {
                throw std::runtime_error("Could not open file: " + file_path);
            }

            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string source = buffer.str();

            Lexer lexer(source);
            auto tokens = lexer.scan_tokens();
            Parser parser(tokens);
            auto ast = parser.parse();

            m_ymeta_manager.load(file_path);
            Resolver resolver(ast, m_ymeta_manager);
            auto nested_config = resolver.resolve();

            Validator validator(nested_config, ast);
            validator.validate();

            // Flatten the (potentially modified) map for the interop layer
            for (const auto& [section_name, section_map] : nested_config) {
                if (section_name.empty()) {
                    for (const auto& [key, value] : section_map) {
                        m_resolved_config[key] = value;
                    }
                } else {
                    for (const auto& [key, value] : section_map) {
                        m_resolved_config[section_name + "." + key] = value;
                    }
                }
            }

            m_ymeta_manager.save(file_path);
        }

        std::map<std::string, std::any> m_resolved_config;
        YmetaManager m_ymeta_manager;
    };
}

YINI_API void* yini_create_from_file(const char* file_path)
{
    try
    {
        YINI::Config* config = new YINI::Config(file_path);
        return static_cast<void*>(config);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error creating YINI config: " << e.what() << std::endl;
        return nullptr;
    }
}

YINI_API void yini_destroy(void* handle)
{
    if (handle)
    {
        delete static_cast<YINI::Config*>(handle);
    }
}

YINI_API bool yini_get_int(void* handle, const char* key, int* out_value)
{
    if (!handle || !key || !out_value) return false;
    YINI::Config* config = static_cast<YINI::Config*>(handle);
    try
    {
        if (config->m_resolved_config.count(key))
        {
            *out_value = static_cast<int>(std::any_cast<double>(config->m_resolved_config.at(key)));
            return true;
        }
    }
    catch (const std::bad_any_cast&) {}
    return false;
}

YINI_API bool yini_get_double(void* handle, const char* key, double* out_value)
{
    if (!handle || !key || !out_value) return false;
    YINI::Config* config = static_cast<YINI::Config*>(handle);
    try
    {
        if (config->m_resolved_config.count(key))
        {
            *out_value = std::any_cast<double>(config->m_resolved_config.at(key));
            return true;
        }
    }
    catch (const std::bad_any_cast&) {}
    return false;
}

YINI_API bool yini_get_bool(void* handle, const char* key, bool* out_value)
{
    if (!handle || !key || !out_value) return false;
    YINI::Config* config = static_cast<YINI::Config*>(handle);
    try
    {
        if (config->m_resolved_config.count(key))
        {
            *out_value = std::any_cast<bool>(config->m_resolved_config.at(key));
            return true;
        }
    }
    catch (const std::bad_any_cast&) {}
    return false;
}

YINI_API int yini_get_string_length(void* handle, const char* key)
{
    if (!handle || !key) return -1;
    YINI::Config* config = static_cast<YINI::Config*>(handle);
    try
    {
        if (config->m_resolved_config.count(key))
        {
            const std::string& str = std::any_cast<const std::string&>(config->m_resolved_config.at(key));
            return static_cast<int>(str.length());
        }
    }
    catch (const std::bad_any_cast&) {}
    return -1;
}

YINI_API int yini_get_string(void* handle, const char* key, char* out_buffer, int buffer_size)
{
    if (!handle || !key || !out_buffer) return -1;
    YINI::Config* config = static_cast<YINI::Config*>(handle);
    try
    {
        if (config->m_resolved_config.count(key))
        {
            const std::string& str = std::any_cast<const std::string&>(config->m_resolved_config.at(key));
            if (static_cast<int>(str.length()) < buffer_size)
            {
                strncpy(out_buffer, str.c_str(), buffer_size);
                return static_cast<int>(str.length());
            }
        }
    }
    catch (const std::bad_any_cast&) {}
    return -1;
}

YINI_API bool yini_get_color(void* handle, const char* key, Yini_Color* out_value)
{
    if (!handle || !key || !out_value) return false;
    YINI::Config* config = static_cast<YINI::Config*>(handle);
    try
    {
        if (config->m_resolved_config.count(key))
        {
            const auto& color = std::any_cast<const YINI::ResolvedColor&>(config->m_resolved_config.at(key));
            out_value->r = color.r;
            out_value->g = color.g;
            out_value->b = color.b;
            return true;
        }
    }
    catch (const std::bad_any_cast&) {}
    return false;
}

YINI_API bool yini_get_coord(void* handle, const char* key, Yini_Coord* out_value)
{
    if (!handle || !key || !out_value) return false;
    YINI::Config* config = static_cast<YINI::Config*>(handle);
    try
    {
        if (config->m_resolved_config.count(key))
        {
            const auto& coord = std::any_cast<const YINI::ResolvedCoord&>(config->m_resolved_config.at(key));
            out_value->x = std::any_cast<double>(coord.x);
            out_value->y = std::any_cast<double>(coord.y);
            out_value->has_z = coord.z.has_value();
            if (out_value->has_z) {
                out_value->z = std::any_cast<double>(coord.z);
            } else {
                out_value->z = 0;
            }
            return true;
        }
    }
    catch (const std::bad_any_cast&) {}
    return false;
}

YINI_API int yini_get_array_size(void* handle, const char* key)
{
    if (!handle || !key) return -1;
    YINI::Config* config = static_cast<YINI::Config*>(handle);
    try
    {
        if (config->m_resolved_config.count(key))
        {
            const auto& vec = std::any_cast<const std::vector<std::any>&>(config->m_resolved_config.at(key));
            return static_cast<int>(vec.size());
        }
    }
    catch (const std::bad_any_cast&) {}
    return -1;
}

YINI_API int yini_get_array_item_as_string_length(void* handle, const char* key, int index)
{
    if (!handle || !key) return -1;
    YINI::Config* config = static_cast<YINI::Config*>(handle);
    try
    {
        if (config->m_resolved_config.count(key))
        {
            const auto& vec = std::any_cast<const std::vector<std::any>&>(config->m_resolved_config.at(key));
            if (index >= 0 && static_cast<size_t>(index) < vec.size())
            {
                const std::string& str = std::any_cast<const std::string&>(vec.at(index));
                return static_cast<int>(str.length());
            }
        }
    }
    catch (const std::bad_any_cast&) {}
    return -1;
}

YINI_API int yini_get_array_item_as_string(void* handle, const char* key, int index, char* out_buffer, int buffer_size)
{
    if (!handle || !key || !out_buffer) return -1;
    YINI::Config* config = static_cast<YINI::Config*>(handle);
    try
    {
        if (config->m_resolved_config.count(key))
        {
            const auto& vec = std::any_cast<const std::vector<std::any>&>(config->m_resolved_config.at(key));
            if (index >= 0 && static_cast<size_t>(index) < vec.size())
            {
                const std::string& str = std::any_cast<const std::string&>(vec.at(index));
                if (static_cast<int>(str.length()) < buffer_size)
                {
                    strncpy(out_buffer, str.c_str(), buffer_size);
                    return static_cast<int>(str.length());
                }
            }
        }
    }
    catch (const std::bad_any_cast&) {}
    return -1;
}