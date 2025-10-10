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
            m_resolved_config = resolver.resolve();

            Validator validator(m_resolved_config, ast);
            validator.validate();

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

YINI_API const char* yini_get_string(void* handle, const char* key)
{
    if (!handle || !key) return nullptr;
    YINI::Config* config = static_cast<YINI::Config*>(handle);
    try
    {
        if (config->m_resolved_config.count(key))
        {
            const std::string& str = std::any_cast<const std::string&>(config->m_resolved_config.at(key));
            char* c_str = new char[str.length() + 1];
            strcpy(c_str, str.c_str());
            return c_str;
        }
    }
    catch (const std::bad_any_cast&) {}
    return nullptr;
}

YINI_API void yini_free_string(const char* str)
{
    if (str)
    {
        delete[] str;
    }
}