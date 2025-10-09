#include "yini_interop.h"
#include "Parser/parser.h"

extern "C" {

YiniConfigHandle yini_parse_file(const char* filepath) {
    Parser* parser = new Parser();
    Config* config = new Config();
    try {
        *config = parser->parseFile(filepath);
        delete parser;
        return static_cast<YiniConfigHandle>(config);
    } catch (...) {
        delete parser;
        delete config;
        return nullptr;
    }
}

const char* yini_get_string(YiniConfigHandle handle, const char* section, const char* key) {
    if (!handle) return nullptr;
    Config* config = static_cast<Config*>(handle);
    if (config->count(section) && (*config)[section].count(key)) {
        const auto& value = (*config)[section][key];
        if (std::holds_alternative<std::string>(value)) {
            // NOTE: This returns a pointer to the internal string data.
            // The C# side MUST copy this string immediately.
            return std::get<std::string>(value).c_str();
        }
    }
    return nullptr;
}

int yini_get_int(YiniConfigHandle handle, const char* section, const char* key, int default_value) {
    if (!handle) return default_value;
    Config* config = static_cast<Config*>(handle);
    if (config->count(section) && (*config)[section].count(key)) {
        const auto& value = (*config)[section][key];
        if (std::holds_alternative<int>(value)) {
            return std::get<int>(value);
        }
    }
    return default_value;
}

double yini_get_double(YiniConfigHandle handle, const char* section, const char* key, double default_value) {
    if (!handle) return default_value;
    Config* config = static_cast<Config*>(handle);
    if (config->count(section) && (*config)[section].count(key)) {
        const auto& value = (*config)[section][key];
        if (std::holds_alternative<double>(value)) {
            return std::get<double>(value);
        }
    }
    return default_value;
}

bool yini_get_bool(YiniConfigHandle handle, const char* section, const char* key, bool default_value) {
    if (!handle) return default_value;
    Config* config = static_cast<Config*>(handle);
    if (config->count(section) && (*config)[section].count(key)) {
        const auto& value = (*config)[section][key];
        if (std::holds_alternative<bool>(value)) {
            return std::get<bool>(value);
        }
    }
    return default_value;
}

void yini_free_config(YiniConfigHandle handle) {
    if (handle) {
        Config* config = static_cast<Config*>(handle);
        delete config;
    }
}

} // extern "C"