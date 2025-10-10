#include "YmetaManager.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include "YiniTypes.h"

using json = nlohmann::json;

namespace YINI
{

namespace
{
    // Forward declaration for recursive calls
    json any_to_json(const std::any& value);

    std::any json_to_any(const json& j);

    json any_to_json(const std::any& value)
    {
        json j;
        if (!value.has_value()) {
            return nullptr;
        }

        const auto& type = value.type();

        if (type == typeid(std::string)) {
            j["type"] = "string";
            j["value"] = std::any_cast<std::string>(value);
        } else if (type == typeid(double)) {
            j["type"] = "double";
            j["value"] = std::any_cast<double>(value);
        } else if (type == typeid(bool)) {
            j["type"] = "bool";
            j["value"] = std::any_cast<bool>(value);
        } else if (type == typeid(std::vector<std::any>)) {
            j["type"] = "vector";
            const auto& vec = std::any_cast<const std::vector<std::any>&>(value);
            json& val_array = j["value"];
            val_array = json::array();
            for (const auto& elem : vec) {
                val_array.push_back(any_to_json(elem));
            }
        } else if (type == typeid(std::map<std::string, std::any>)) {
            j["type"] = "map";
            const auto& map = std::any_cast<const std::map<std::string, std::any>&>(value);
            json& val_map = j["value"];
            val_map = json::object();
            for (const auto& [key, val] : map) {
                val_map[key] = any_to_json(val);
            }
        } else if (type == typeid(ResolvedColor)) {
            j["type"] = "color";
            const auto& color = std::any_cast<ResolvedColor>(value);
            j["value"] = {{"r", color.r}, {"g", color.g}, {"b", color.b}};
        } else if (type == typeid(ResolvedCoord)) {
            j["type"] = "coord";
            const auto& coord = std::any_cast<ResolvedCoord>(value);
            json& val_coord = j["value"];
            val_coord["x"] = any_to_json(coord.x);
            val_coord["y"] = any_to_json(coord.y);
            if (coord.z.has_value()) {
                val_coord["z"] = any_to_json(coord.z);
            }
        } else {
            j["type"] = "unsupported";
        }
        return j;
    }

    std::any json_to_any(const json& j)
    {
        std::string type = j.at("type").get<std::string>();
        const json& value = j.at("value");

        if (type == "string") return value.get<std::string>();
        if (type == "double") return value.get<double>();
        if (type == "bool") return value.get<bool>();
        if (type == "vector") {
            std::vector<std::any> vec;
            for (const auto& elem : value) {
                vec.push_back(json_to_any(elem));
            }
            return vec;
        }
        if (type == "map") {
            std::map<std::string, std::any> map;
            for (auto& [key, val] : value.items()) {
                map[key] = json_to_any(val);
            }
            return map;
        }
        if (type == "color") {
            ResolvedColor color;
            color.r = value.at("r").get<uint8_t>();
            color.g = value.at("g").get<uint8_t>();
            color.b = value.at("b").get<uint8_t>();
            return color;
        }
        if (type == "coord") {
            ResolvedCoord coord;
            coord.x = json_to_any(value.at("x"));
            coord.y = json_to_any(value.at("y"));
            if (value.contains("z")) {
                coord.z = json_to_any(value.at("z"));
            }
            return coord;
        }
        return {};
    }
}

YmetaManager::YmetaManager()
{
}

void YmetaManager::load(const std::string& yini_filepath)
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
    catch (json::parse_error& e)
    {
        std::cerr << "Warning: Could not parse .ymeta file: " << ymeta_path << ". " << e.what() << std::endl;
        return;
    }

    if (root.contains("dynamic_values"))
    {
        const json& dynamic_values_json = root["dynamic_values"];
        for (auto& [key, value] : dynamic_values_json.items())
        {
            m_dynamic_values[key] = json_to_any(value);
        }
    }

    if (root.contains("backup_values"))
    {
        const json& backup_values_json = root["backup_values"];
        for (auto& [key, values] : backup_values_json.items())
        {
            std::vector<std::any> backup_vec;
            for (const auto& val_json : values)
            {
                backup_vec.push_back(json_to_any(val_json));
            }
            m_backup_values[key] = backup_vec;
        }
    }
}

void YmetaManager::save(const std::string& yini_filepath)
{
    std::string ymeta_path = get_ymeta_path(yini_filepath);
    json root;

    json dynamic_values_json;
    for (const auto& [key, value] : m_dynamic_values)
    {
        dynamic_values_json[key] = any_to_json(value);
    }
    root["dynamic_values"] = dynamic_values_json;

    json backup_values_json;
    for (const auto& [key, values] : m_backup_values)
    {
        json backup_array = json::array();
        for (const auto& val : values)
        {
            backup_array.push_back(any_to_json(val));
        }
        backup_values_json[key] = backup_array;
    }
    root["backup_values"] = backup_values_json;

    std::ofstream file(ymeta_path);
    file << root.dump(4);
}

bool YmetaManager::has_value(const std::string& key)
{
    return m_dynamic_values.count(key);
}

std::any YmetaManager::get_value(const std::string& key)
{
    return m_dynamic_values[key];
}

void YmetaManager::set_value(const std::string& key, std::any value)
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

std::string YmetaManager::get_ymeta_path(const std::string& yini_filepath)
{
    std::filesystem::path p(yini_filepath);
    p.replace_extension(".ymeta");
    return p.string();
}

} // namespace YINI