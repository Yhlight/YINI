#pragma once

#include <string>
#include <map>
#include <any>
#include <fstream>
#include <vector>

namespace YINI
{
    namespace Serialization
    {
        class Deserializer
        {
        public:
            std::map<std::string, std::map<std::string, std::any>> deserialize(const std::string& filepath);

        private:
            std::any read_any(std::ifstream& in, const std::string& filepath);
            std::string read_string(std::ifstream& in);
        };
    }
}