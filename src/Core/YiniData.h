#pragma once

#include "YiniSection.h"
#include <string>
#include <map>

namespace Yini
{
    class YiniData
    {
    public:
        YiniData() = default;

        void addSection(const YiniSection& section);
        YiniSection* getSection(const std::string& sectionName);
        const YiniSection* getSection(const std::string& sectionName) const;

        std::map<std::string, YiniSection>& getSections();
        const std::map<std::string, YiniSection>& getSections() const;

        void addMacro(const std::string& key, const YiniValue& value);
        const YiniMap& getMacros() const;

        void addInclude(const std::string& filepath);
        const std::vector<std::string>& getIncludes() const;

    private:
        std::map<std::string, YiniSection> m_sections;
        YiniMap m_macros;
        std::vector<std::string> m_includes;
    };
}
