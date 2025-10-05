#pragma once

#pragma once

#include "Core/YiniValue.h"
#include <string>
#include <fstream>
#include <map>

namespace YINI
{
    namespace Serialization
    {
        // Forward-declare the visitor so it can be friended.
        struct SerializeVisitor;

        class Serializer
        {
            // Grant the visitor access to private methods.
            friend struct SerializeVisitor;

        public:
            void serialize(const std::map<std::string, std::map<std::string, YiniValue, std::less<>>, std::less<>>& data, const std::string& filepath);

        private:
            void write_value(std::ofstream& out, const YiniValue& value);
            void write_string(std::ofstream& out, const std::string& str);
        };
    }
}