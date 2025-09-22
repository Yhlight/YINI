#pragma once

#include "YiniData.h"
#include <string>
#include <set>

namespace Yini
{
    class YiniLoader
    {
    public:
        YiniLoader();

        YiniData loadFile(const std::string& filepath);

    private:
        YiniData loadFileRecursive(const std::string& filepath, std::set<std::string>& loadedFiles);
        YiniData loadAndParse(const std::string& filepath);
        void resolveIncludes(YiniData& data, const std::string& basePath, std::set<std::string>& loadedFiles);
        void resolveInheritance(YiniData& data);
        void resolveMacros(YiniData& data);
        void mergeData(YiniData& base, const YiniData& child);
    };
}
