#ifndef YMETA_MANAGER_H
#define YMETA_MANAGER_H

#include "Parser/parser.h"
#include <string>

class YmetaManager {
public:
    void write(const std::string& yini_filepath, const Config& config);
};

#endif // YMETA_MANAGER_H