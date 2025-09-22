#pragma once

#include "YiniData.h"
#include <string>

namespace Yini
{
    class YmetaSerializer
    {
    public:
        YmetaSerializer();

        bool save(const YiniData& data, const std::string& filepath);
        YiniData load(const std::string& filepath);
    };
}
