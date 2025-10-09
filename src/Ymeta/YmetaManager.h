#ifndef YMETA_MANAGER_H
#define YMETA_MANAGER_H

#include "Parser/parser.h"
#include <string>
#include <optional>

class YmetaManager {
public:
    void write(const std::string& yini_filepath, const Config& config);
    std::optional<Config> read(const std::string& yini_filepath);
    void write_yini(const std::string& yini_filepath, const Config& config);
};

#endif // YMETA_MANAGER_H