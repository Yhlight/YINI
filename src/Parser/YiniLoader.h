#pragma once

#include "Ast.h"
#include <string>
#include <set>

namespace Yini
{
    class Loader
    {
    public:
        YiniFile load(const std::string& rootFilepath);

    private:
        YiniFile parseFile(const std::string& filepath);
        void processIncludes(YiniFile& ast, const std::string& basePath, std::set<std::string>& processedFiles);
        void merge(YiniFile& baseAst, const YiniFile& includedAst);
        void resolveMacros(YiniFile& ast);
        void applyInheritance(YiniFile& ast);
        void applySectionInheritance(YiniSection& section, YiniFile& ast, std::set<std::string>& inheritanceChain);
    };
} // namespace Yini
