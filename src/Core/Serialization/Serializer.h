#pragma once

#include "Interpreter/Interpreter.h"
#include <string>
#include <fstream>

namespace YINI
{
    namespace Serialization
    {
        class Serializer
        {
        public:
            void serialize(const std::map<std::string, std::map<std::string, std::any>>& data, const std::string& filepath);

        private:
            void write_any(std::ofstream& out, const std::any& value);
            void write_string(std::ofstream& out, const std::string& str);
        };
    }
}