#ifndef YINI_JSON_SERIALIZER_HPP
#define YINI_JSON_SERIALIZER_HPP

#include "YiniData.hpp"
#include <string>

namespace YINI
{
    class JsonSerializer
    {
    public:
        static std::string serialize(const YiniDocument& doc);
    };
}

#endif // YINI_JSON_SERIALIZER_HPP