#include "YiniData.h"

namespace Yini
{
    void YiniData::addSection(const YiniSection& section)
    {
        m_sections[section.getName()] = section;
    }

    YiniSection* YiniData::getSection(const std::string& sectionName)
    {
        auto it = m_sections.find(sectionName);
        if (it != m_sections.end())
        {
            return &it->second;
        }
        return nullptr;
    }

    const YiniSection* YiniData::getSection(const std::string& sectionName) const
    {
        auto it = m_sections.find(sectionName);
        if (it != m_sections.end())
        {
            return &it->second;
        }
        return nullptr;
    }

    std::map<std::string, YiniSection>& YiniData::getSections()
    {
        return m_sections;
    }

    const std::map<std::string, YiniSection>& YiniData::getSections() const
    {
        return m_sections;
    }

    void YiniData::addMacro(const std::string& key, const YiniValue& value)
    {
        m_macros[key] = value;
    }

    const YiniMap& YiniData::getMacros() const
    {
        return m_macros;
    }

    void YiniData::addInclude(const std::string& filepath)
    {
        m_includes.push_back(filepath);
    }

    const std::vector<std::string>& YiniData::getIncludes() const
    {
        return m_includes;
    }
}
