#include "yini_interop.h"
#include "Parser/parser.h"
#include "Ymeta/YmetaManager.h"

// This is a C++ template and must be outside the extern "C" block.
template<typename T>
void yini_set_value_internal(YiniConfigHandle handle, const char* section, const char* key, T new_value) {
    if (!handle) return;
    Config* config = static_cast<Config*>(handle);

    auto& value_variant = (*config)[section][key];

    if (std::holds_alternative<std::unique_ptr<DynaValue>>(value_variant)) {
        auto* dyna_ptr = std::get<std::unique_ptr<DynaValue>>(value_variant).get();
        if (dyna_ptr) {
            // Add current value to backup, maintaining a max size of 5
            if (dyna_ptr->value) {
                if (dyna_ptr->backup.size() >= 5) {
                    dyna_ptr->backup.erase(dyna_ptr->backup.begin());
                }
                dyna_ptr->backup.push_back(std::move(dyna_ptr->value));
            }
            // Set the new value
            dyna_ptr->value = std::make_unique<ConfigValue>(new_value);
        } else {
             // If dyna_ptr is null, create a new one
            auto new_dyna_val = std::make_unique<DynaValue>();
            new_dyna_val->value = std::make_unique<ConfigValue>(new_value);
            value_variant = std::move(new_dyna_val);
        }
    } else {
        // Not a dynamic value, just overwrite it
        value_variant = new_value;
    }
}


extern "C" {

// --- Lifecycle ---
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

void yini_save_file(YiniConfigHandle handle, const char* filepath) {
    if (!handle) return;
    Config* config = static_cast<Config*>(handle);
    YmetaManager ymeta_manager;
    ymeta_manager.write_yini(filepath, *config);
}

void yini_free_config(YiniConfigHandle handle) {
    if (handle) {
        Config* config = static_cast<Config*>(handle);
        delete config;
    }
}

// --- Primitive Getters ---
const char* yini_get_string(YiniConfigHandle handle, const char* section, const char* key) {
    if (!handle) return nullptr;
    Config* config = static_cast<Config*>(handle);
    if (config->count(section) && (*config)[section].count(key)) {
        const auto& value = (*config)[section][key];
        if (std::holds_alternative<std::string>(value)) {
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

// --- Primitive Setters ---
void yini_set_string(YiniConfigHandle handle, const char* section, const char* key, const char* value) {
    yini_set_value_internal(handle, section, key, std::string(value));
}

void yini_set_int(YiniConfigHandle handle, const char* section, const char* key, int value) {
    yini_set_value_internal(handle, section, key, value);
}

void yini_set_double(YiniConfigHandle handle, const char* section, const char* key, double value) {
    yini_set_value_internal(handle, section, key, value);
}

void yini_set_bool(YiniConfigHandle handle, const char* section, const char* key, bool value) {
    yini_set_value_internal(handle, section, key, value);
}


// --- Complex Value Getters ---
YiniValue* to_c_style_value(const ConfigValue& cpp_value); // Forward declaration

YiniValue* yini_get_value(YiniConfigHandle handle, const char* section, const char* key) {
    if (!handle) return nullptr;
    Config* config = static_cast<Config*>(handle);
    if (config->count(section) && (*config)[section].count(key)) {
        const auto& value = (*config)[section][key];
        return to_c_style_value(value);
    }
    return nullptr;
}

void yini_free_value(YiniValue* value) {
    if (!value) return;

    switch (value->type) {
        case YINI_TYPE_STRING:
            delete[] value->as.string_value;
            break;
        case YINI_TYPE_ARRAY:
            for (size_t i = 0; i < value->as.array_value->size; ++i) {
                yini_free_value(value->as.array_value->elements[i]);
            }
            delete[] value->as.array_value->elements;
            delete value->as.array_value;
            break;
        case YINI_TYPE_MAP:
            for (size_t i = 0; i < value->as.map_value->size; ++i) {
                delete[] value->as.map_value->entries[i]->key;
                yini_free_value(value->as.map_value->entries[i]->value);
                delete value->as.map_value->entries[i];
            }
            delete[] value->as.map_value->entries;
            delete value->as.map_value;
            break;
        default:
            // No memory to free for primitive types or NULL
            break;
    }
    delete value;
}

YiniValue* to_c_style_value(const ConfigValue& cpp_value) {
    YiniValue* c_value = new YiniValue();

    std::visit([&](const auto& v) {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, std::string>) {
            c_value->type = YINI_TYPE_STRING;
            char* str = new char[v.length() + 1];
            strcpy(str, v.c_str());
            c_value->as.string_value = str;
        } else if constexpr (std::is_same_v<T, int>) {
            c_value->type = YINI_TYPE_INT;
            c_value->as.int_value = v;
        } else if constexpr (std::is_same_v<T, double>) {
            c_value->type = YINI_TYPE_DOUBLE;
            c_value->as.double_value = v;
        } else if constexpr (std::is_same_v<T, bool>) {
            c_value->type = YINI_TYPE_BOOL;
            c_value->as.bool_value = v;
        } else if constexpr (std::is_same_v<T, std::unique_ptr<Array>>) {
            c_value->type = YINI_TYPE_ARRAY;
            YiniArray* c_array = new YiniArray();
            c_array->size = v->elements.size();
            c_array->elements = new YiniValue*[c_array->size];
            for (size_t i = 0; i < c_array->size; ++i) {
                c_array->elements[i] = to_c_style_value(v->elements[i]);
            }
            c_value->as.array_value = c_array;
        } else if constexpr (std::is_same_v<T, std::unique_ptr<Map>>) {
            c_value->type = YINI_TYPE_MAP;
            YiniMap* c_map = new YiniMap();
            c_map->size = v->elements.size();
            c_map->entries = new YiniMapEntry*[c_map->size];
            size_t i = 0;
            for (const auto& [key, value] : v->elements) {
                YiniMapEntry* entry = new YiniMapEntry();
                char* c_key = new char[key.length() + 1];
                strcpy(c_key, key.c_str());
                entry->key = c_key;
                entry->value = to_c_style_value(value);
                c_map->entries[i++] = entry;
            }
            c_value->as.map_value = c_map;
        } else {
             c_value->type = YINI_TYPE_NULL;
        }
    }, cpp_value);

    return c_value;
}

} // extern "C"