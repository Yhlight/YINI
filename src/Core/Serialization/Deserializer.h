#pragma once

#pragma once

#include <fstream>
#include <map>
#include <string>
#include <vector>

#include "Core/YiniValue.h"

namespace YINI {
namespace Serialization {
class Deserializer {
public:
    std::map<std::string, std::map<std::string, YiniValue, std::less<>>, std::less<>> deserialize(
        const std::string& filepath);

private:
    YiniValue read_value(std::ifstream& in);
    std::string read_string(std::ifstream& in);
};
}  // namespace Serialization
}  // namespace YINI