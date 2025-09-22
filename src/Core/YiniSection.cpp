#include "YiniSection.h"

namespace Yini
{
    YiniSection::YiniSection(const std::string& name) : m_name(name)
    {
    }

    const std::string& YiniSection::getName() const
    {
        return m_name;
    }

    void YiniSection::addInheritance(const std::string& sectionName)
    {
        m_inherits.push_back(sectionName);
    }

    const std::vector<std::string>& YiniSection::getInheritance() const
    {
        return m_inherits;
    }

    void YiniSection::addValue(const YiniValue& value)
    {
        m_values.push_back(value);
    }

    void YiniSection::addKeyValuePair(const std::string& key, const YiniValue& value)
    {
        m_keyValues[key] = value;
    }

    const std::vector<YiniValue>& YiniSection::getValues() const
    {
        return m_values;
    }

    const std::map<std::string, YiniValue>& YiniSection::getKeyValues() const
    {
        return m_keyValues;
    }
}
