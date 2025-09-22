#pragma once

#include "YiniValue.h"
#include <string>
#include <vector>
#include <map>

namespace Yini
{
    class YiniSection
    {
    public:
        YiniSection() = default;
        YiniSection(const std::string& name);

        const std::string& getName() const;
        void addInheritance(const std::string& sectionName);
        const std::vector<std::string>& getInheritance() const;

        void addValue(const YiniValue& value);
        void addKeyValuePair(const std::string& key, const YiniValue& value);

        const std::vector<YiniValue>& getValues() const;
        const std::map<std::string, YiniValue>& getKeyValues() const;

    private:
        std::string m_name;
        std::vector<std::string> m_inherits;
        std::map<std::string, YiniValue> m_keyValues;
        std::vector<YiniValue> m_values; // For += syntax
    };
}
