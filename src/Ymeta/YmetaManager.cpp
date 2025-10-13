#include "YmetaManager.h"
#include "YiniTypes.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace YINI
{

namespace
{
    // Forward declaration for recursive calls
    json variant_to_json(const YiniVariant &value);
    YiniVariant json_to_variant(const json &j);

    json variant_to_json(const YiniVariant &value)
    {
        json j;
        std::visit([&j](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::monostate>) {
                j["type"] = "null";
                j["value"] = nullptr;
            } else if constexpr (std::is_same_v<T, int64_t>) {
                j["type"] = "int";
                j["value"] = arg;
            } else if constexpr (std::is_same_v<T, double>) {
                j["type"] = "double";
                j["value"] = arg;
            } else if constexpr (std::is_same_v<T, bool>) {
                j["type"] = "bool";
                j["value"] = arg;
            } else if constexpr (std::is_same_v<T, std::string>) {
                j["type"] = "string";
                j["value"] = arg;
            } else if constexpr (std::is_same_v<T, ResolvedColor>) {
                j["type"] = "color";
                j["value"] = {{"r", arg.r}, {"g", arg.g}, {"b", arg.b}};
            } else if constexpr (std::is_same_v<T, ResolvedCoord>) {
                j["type"] = "coord";
                j["value"] = {{"x", arg.x}, {"y", arg.y}, {"z", arg.z}, {"has_z", arg.has_z}};
            } else if constexpr (std::is_same_v<T, std::unique_ptr<YiniArray>>) {
                j["type"] = "array";
                json& val_array = j["value"];
                val_array = json::array();
                if (arg) {
                    for (const auto &elem : *arg) {
                        val_array.push_back(variant_to_json(elem));
                    }
                }
            } else if constexpr (std::is_same_v<T, YiniMap>) {
                 j["type"] = "map";
                json& val_map = j["value"];
                val_map = json::object();
                for (const auto &[key, val] : arg) {
                    val_map[key] = variant_to_json(val);
                }
            } else if constexpr (std::is_same_v<T, YiniStruct>) {
                j["type"] = "struct";
                json& val_struct = j["value"];
                val_struct[arg.first] = variant_to_json(*arg.second);
            }
        }, value);
        return j;
    }

    YiniVariant json_to_variant(const json &j)
    {
        std::string type = j.at("type").get<std::string>();
        const json &value = j.at("value");

        if (type == "null") return std::monostate{};
        if (type == "int") return value.get<int64_t>();
        if (type == "double") return value.get<double>();
        if (type == "bool") return value.get<bool>();
        if (type == "string") return value.get<std::string>();
        if (type == "color") {
            ResolvedColor color;
            color.r = value.at("r").get<uint8_t>();
            color.g = value.at("g").get<uint8_t>();
            color.b = value.at("b").get<uint8_t>();
            return color;
        }
        if (type == "coord") {
            ResolvedCoord coord;
            coord.x = value.at("x").get<double>();
            coord.y = value.at("y").get<double>();
            coord.z = value.at("z").get<double>();
            coord.has_z = value.at("has_z").get<bool>();
            return coord;
        }
        if (type == "array") {
            auto arr = std::make_unique<YiniArray>();
            for (const auto &elem : value) {
                arr->push_back(json_to_variant(elem));
            }
            return arr;
        }
        if (type == "map") {
            YiniMap map;
            for (auto &[key, val] : value.items()) {
                map[key] = json_to_variant(val);
            }
            return map;
        }
        if (type == "struct") {
            YiniStruct yini_struct;
            for (auto &[key, val] : value.items()) {
                yini_struct.first = key;
                yini_struct.second = std::make_unique<YiniVariant>(json_to_variant(val));
            }
            return yini_struct;
        }
        return std::monostate{};
    }
} // namespace

YmetaManager::YmetaManager() {}

void YmetaManager::load(const std::string &yini_filepath)
{
    std::string ymeta_path = get_ymeta_path(yini_filepath);
    if (!std::filesystem::exists(ymeta_path))
    {
        return;
    }

    std::ifstream file(ymeta_path);
    json root;
    try
    {
        file >> root;
    }
    catch (json::parse_error &e)
    {
        std::cerr << "Warning: Could not parse .ymeta file: " << ymeta_path << ". " << e.what() << std::endl;
        return;
    }

    if (root.contains("dynamic_values"))
    {
        const json &dynamic_values_json = root["dynamic_values"];
        for (auto &[key, value] : dynamic_values_json.items())
        {
            m_dynamic_values[key] = json_to_variant(value);
        }
    }

    if (root.contains("backup_values"))
    {
        const json &backup_values_json = root["backup_values"];
        for (auto &[key, values] : backup_values_json.items())
        {
            std::vector<YiniVariant> backup_vec;
            for (const auto &val_json : values)
            {
                backup_vec.push_back(json_to_variant(val_json));
            }
            m_backup_values[key] = backup_vec;
        }
    }
}

void YmetaManager::save(const std::string &yini_filepath)
{
    std::string ymeta_path = get_ymeta_path(yini_filepath);
    json root;

    json dynamic_values_json;
    for (const auto &[key, value] : m_dynamic_values)
    {
        dynamic_values_json[key] = variant_to_json(value);
    }
    root["dynamic_values"] = dynamic_values_json;

    json backup_values_json;
    for (const auto &[key, values] : m_backup_values)
    {
        json backup_array = json::array();
        for (const auto &val : values)
        {
            backup_array.push_back(variant_to_json(val));
        }
        backup_values_json[key] = backup_array;
    }
    root["backup_values"] = backup_values_json;

    std::ofstream file(ymeta_path);
    file << root.dump(4);
}

bool YmetaManager::has_value(const std::string &key)
{
    return m_dynamic_values.count(key);
}

YiniVariant YmetaManager::get_value(const std::string &key)
{
    return m_dynamic_values[key];
}

void YmetaManager::set_value(const std::string &key, YiniVariant value)
{
    if (m_dynamic_values.count(key))
    {
        m_backup_values[key].push_back(m_dynamic_values[key]);
        if (m_backup_values[key].size() > 5)
        {
            m_backup_values[key].erase(m_backup_values[key].begin());
        }
    }
    m_dynamic_values[key] = value;
}

std::string YmetaManager::get_ymeta_path(const std::string &yini_filepath)
{
    std::filesystem::path p(yini_filepath);
    p.replace_extension(".ymeta");
    return p.string();
}

} // namespace YINI
