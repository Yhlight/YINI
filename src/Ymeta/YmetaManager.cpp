#include "YmetaManager.h"
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>

void YmetaManager::write(const std::string& yini_filepath, const Config& config) {
    std::filesystem::path path(yini_filepath);
    path.replace_extension(".ymeta");

    nlohmann::json j = config;

    std::ofstream ymeta_file(path);
    ymeta_file << j.dump(4);
}

std::optional<Config> YmetaManager::read(const std::string& yini_filepath) {
    std::filesystem::path yini_path(yini_filepath);
    std::filesystem::path ymeta_path = yini_path;
    ymeta_path.replace_extension(".ymeta");

    if (std::filesystem::exists(ymeta_path) && std::filesystem::exists(yini_path)) {
        auto yini_time = std::filesystem::last_write_time(yini_path);
        auto ymeta_time = std::filesystem::last_write_time(ymeta_path);

        if (ymeta_time >= yini_time) {
            std::ifstream ymeta_file(ymeta_path);
            nlohmann::json j;
            ymeta_file >> j;
            return j.get<Config>();
        }
    }

    return std::nullopt;
}

void YmetaManager::write_yini(const std::string& yini_filepath, const Config& config) {
    std::stringstream ss;
    for (const auto& [section_name, section] : config) {
        ss << "[" << section_name << "]\n";
        for (const auto& [key, value] : section) {
            ss << key << " = " << to_yini_string(value) << "\n";
        }
        ss << "\n";
    }

    std::ofstream yini_file(yini_filepath);
    yini_file << ss.str();
}