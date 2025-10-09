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
    Config* config = new Config();
    try {
        YmetaManager ymeta_manager;
        auto cached_config = ymeta_manager.read(filepath);
        if (cached_config) {
            *config = std::move(*cached_config);
        } else {
            Parser parser;
            *config = parser.parseFile(filepath);
        }
        return static_cast<YiniConfigHandle>(config);
    } catch (...) {
        delete config;
        return nullptr;
    }
}

void yini_save_file(YiniConfigHandle handle, const char* filepath) {
    if (!handle) return;
    Config* config = static_cast<Config*>(handle);
    YmetaManager ymeta_manager;
    ymeta_manager.write_yini(filepath, *config);
    ymeta_manager.write(filepath, *config); // Also save the ymeta cache
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
        const auto& value_variant = (*config)[section][key];
        if (std::holds_alternative<std::string>(value_variant)) {
            return std::get<std::string>(value_variant).c_str();
        }
        if (std::holds_alternative<std::unique_ptr<DynaValue>>(value_variant)) {
            const auto* dyna_ptr = std::get<std::unique_ptr<DynaValue>>(value_variant).get();
            if (dyna_ptr && dyna_ptr->value && std::holds_alternative<std::string>(*dyna_ptr->value)) {
                return std::get<std::string>(*dyna_ptr->value).c_str();
            }
        }
    }
    return nullptr;
}

int yini_get_int(YiniConfigHandle handle, const char* section, const char* key, int default_value) {
    if (!handle) return default_value;
    Config* config = static_cast<Config*>(handle);
    if (config->count(section) && (*config)[section].count(key)) {
        const auto& value_variant = (*config)[section][key];
        if (std::holds_alternative<int>(value_variant)) {
            return std::get<int>(value_variant);
        }
        if (std::holds_alternative<std::unique_ptr<DynaValue>>(value_variant)) {
            const auto* dyna_ptr = std::get<std::unique_ptr<DynaValue>>(value_variant).get();
            if (dyna_ptr && dyna_ptr->value && std::holds_alternative<int>(*dyna_ptr->value)) {
                return std::get<int>(*dyna_ptr->value);
            }
        }
    }
    return default_value;
}

double yini_get_double(YiniConfigHandle handle, const char* section, const char* key, double default_value) {
    if (!handle) return default_value;
    Config* config = static_cast<Config*>(handle);
    if (config->count(section) && (*config)[section].count(key)) {
        const auto& value_variant = (*config)[section][key];
        if (std::holds_alternative<double>(value_variant)) {
            return std::get<double>(value_variant);
        }
        if (std::holds_alternative<std::unique_ptr<DynaValue>>(value_variant)) {
            const auto* dyna_ptr = std::get<std::unique_ptr<DynaValue>>(value_variant).get();
            if (dyna_ptr && dyna_ptr->value && std::holds_alternative<double>(*dyna_ptr->value)) {
                return std::get<double>(*dyna_ptr->value);
            }
        }
    }
    return default_value;
}

bool yini_get_bool(YiniConfigHandle handle, const char* section, const char* key, bool default_value) {
    if (!handle) return default_value;
    Config* config = static_cast<Config*>(handle);
    if (config->count(section) && (*config)[section].count(key)) {
        const auto& value_variant = (*config)[section][key];
        if (std::holds_alternative<bool>(value_variant)) {
            return std::get<bool>(value_variant);
        }
        if (std::holds_alternative<std::unique_ptr<DynaValue>>(value_variant)) {
            const auto* dyna_ptr = std::get<std::unique_ptr<DynaValue>>(value_variant).get();
            if (dyna_ptr && dyna_ptr->value && std::holds_alternative<bool>(*dyna_ptr->value)) {
                return std::get<bool>(*dyna_ptr->value);
            }
        }
    }
    return default_value;
}

YiniColor yini_get_color(YiniConfigHandle handle, const char* section, const char* key, YiniColor default_value) {
    if (!handle) return default_value;
    Config* config = static_cast<Config*>(handle);
    if (config->count(section) && (*config)[section].count(key)) {
        const auto& value_variant = (*config)[section][key];
        if (std::holds_alternative<Color>(value_variant)) {
            const auto& color = std::get<Color>(value_variant);
            return {color.r, color.g, color.b};
        }
    }
    return default_value;
}

YiniCoord yini_get_coord(YiniConfigHandle handle, const char* section, const char* key, YiniCoord default_value) {
    if (!handle) return default_value;
    Config* config = static_cast<Config*>(handle);
    if (config->count(section) && (*config)[section].count(key)) {
        const auto& value_variant = (*config)[section][key];
        if (std::holds_alternative<Coord>(value_variant)) {
            const auto& coord = std::get<Coord>(value_variant);
            return {coord.x, coord.y, coord.z};
        }
    }
    return default_value;
}

const char* yini_get_path(YiniConfigHandle handle, const char* section, const char* key) {
    if (!handle) return nullptr;
    Config* config = static_cast<Config*>(handle);
    if (config->count(section) && (*config)[section].count(key)) {
        const auto& value_variant = (*config)[section][key];
        if (std::holds_alternative<Path>(value_variant)) {
            return std::get<Path>(value_variant).value.c_str();
        }
    }
    return nullptr;
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

void yini_set_color(YiniConfigHandle handle, const char* section, const char* key, YiniColor value) {
    yini_set_value_internal(handle, section, key, Color{value.r, value.g, value.b});
}

void yini_set_coord(YiniConfigHandle handle, const char* section, const char* key, YiniCoord value) {
    yini_set_value_internal(handle, section, key, Coord{value.x, value.y, value.z});
}

void yini_set_path(YiniConfigHandle handle, const char* section, const char* key, const char* value) {
    yini_set_value_internal(handle, section, key, Path{std::string(value)});
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
            const size_t len = v.length();
            char* str = new char[len + 1];
            strncpy(str, v.c_str(), len);
            str[len] = '\0';
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
                const size_t key_len = key.length();
                char* c_key = new char[key_len + 1];
                strncpy(c_key, key.c_str(), key_len);
                c_key[key_len] = '\0';
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

YiniDynaValue* yini_get_dyna(YiniConfigHandle handle, const char* section, const char* key) {
    if (!handle) return nullptr;
    Config* config = static_cast<Config*>(handle);

    if (config->count(section) && (*config)[section].count(key)) {
        const auto& value_variant = (*config)[section][key];
        if (std::holds_alternative<std::unique_ptr<DynaValue>>(value_variant)) {
            const auto* dyna_ptr = std::get<std::unique_ptr<DynaValue>>(value_variant).get();
            if (dyna_ptr) {
                YiniDynaValue* c_dyna = new YiniDynaValue();
                c_dyna->value = dyna_ptr->value ? to_c_style_value(*dyna_ptr->value) : nullptr;

                YiniArray* c_backups = new YiniArray();
                c_backups->size = dyna_ptr->backup.size();
                c_backups->elements = new YiniValue*[c_backups->size];
                for (size_t i = 0; i < c_backups->size; ++i) {
                    c_backups->elements[i] = to_c_style_value(*dyna_ptr->backup[i]);
                }
                c_dyna->backups = c_backups;

                return c_dyna;
            }
        }
    }
    return nullptr;
}

void yini_free_dyna(YiniDynaValue* dyna) {
    if (!dyna) return;
    yini_free_value(dyna->value);
    if (dyna->backups) {
        for (size_t i = 0; i < dyna->backups->size; ++i) {
            yini_free_value(dyna->backups->elements[i]);
        }
        delete[] dyna->backups->elements;
        delete dyna->backups;
    }
    delete dyna;
}

} // extern "C"