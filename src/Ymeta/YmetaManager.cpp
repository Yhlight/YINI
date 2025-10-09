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