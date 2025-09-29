#ifndef YINI_SERIALIZER_HPP
#define YINI_SERIALIZER_HPP

#include "YiniData.hpp"
#include <string>

namespace YINI
{
    class YiniSerializer
    {
    public:
        static std::string serialize(const YiniDocument& doc);
    };
}

#endif // YINI_SERIALIZER_HPP