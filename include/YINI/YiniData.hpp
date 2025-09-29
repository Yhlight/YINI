#ifndef YINI_DATA_HPP
#define YINI_DATA_HPP

#include <string>
#include <vector>
#include <variant>
#include <map>
#include <memory>
#include <algorithm>

namespace YINI
{
// Forward declarations
struct YiniValue;
struct YiniArray;
struct YiniMap;
struct YiniDynaValue;
struct YiniCoord;
struct YiniColor;
struct YiniPath;

// YiniVariant holds the possible types. Recursive types are held by unique_ptr.
using YiniVariant = std::variant<std::string, int, double, bool, std::unique_ptr<YiniArray>, std::unique_ptr<YiniMap>,
                                 std::unique_ptr<YiniDynaValue>, std::unique_ptr<YiniCoord>, std::unique_ptr<YiniColor>,
                                 std::unique_ptr<YiniPath>>;

// YiniValue is a wrapper around the variant to enable recursion.
struct YiniValue
{
    YiniVariant data;

    // Constructors
    YiniValue();
    YiniValue(const YiniValue &other);
    YiniValue(YiniValue &&other) noexcept;

    // Assignment operators
    YiniValue &operator=(const YiniValue &other);
    YiniValue &operator=(YiniValue &&other) noexcept;

    // Destructor
    ~YiniValue();
};

// YiniArray contains a vector of YiniValues.
struct YiniArray
{
    std::vector<YiniValue> elements;
};

// YiniMap contains a map of string to YiniValue.
struct YiniMap
{
    std::map<std::string, YiniValue> elements;
};

// YiniDynaValue wraps another YiniValue.
struct YiniDynaValue
{
    YiniValue value;
};

struct YiniCoord
{
    double x, y, z;
    bool is_3d;
};

struct YiniColor
{
    unsigned char r, g, b;
};

struct YiniPath
{
    std::string path_value;
};

struct YiniKeyValuePair
{
    std::string key;
    YiniValue value;
};

struct YiniSection
{
    std::string name;
    std::vector<std::string> inheritedSections;
    std::vector<YiniKeyValuePair> pairs;
    std::vector<YiniValue> registrationList;
};

class YiniDocument
{
  public:
    void addSection(const YiniSection &section)
    {
        sections.push_back(section);
    }

    void addSection(YiniSection &&section)
    {
        sections.push_back(std::move(section));
    }

    std::vector<YiniSection> &getSections()
    {
        return sections;
    }

    const std::vector<YiniSection> &getSections() const
    {
        return sections;
    }

  public:
    YiniSection *findSection(const std::string &name)
    {
        auto it = std::find_if(sections.begin(), sections.end(), [&](const YiniSection &s) { return s.name == name; });

        if (it != sections.end())
        {
            return &(*it);
        }

        return nullptr;
    }

    void addDefine(const std::string &key, const YiniValue &value)
    {
        defines[key] = value;
    }

    bool getDefine(const std::string &key, YiniValue &value) const
    {
        auto it = defines.find(key);
        if (it != defines.end())
        {
            value = it->second;
            return true;
        }
        return false;
    }

  public:
    const YiniSection *findSection(const std::string &name) const
    {
        auto it = std::find_if(sections.begin(), sections.end(), [&](const YiniSection &s) { return s.name == name; });

        if (it != sections.end())
        {
            return &(*it);
        }

        return nullptr;
    }

    YiniSection *getOrCreateSection(const std::string &name)
    {
        auto it = std::find_if(sections.begin(), sections.end(), [&](const YiniSection &s) { return s.name == name; });

        if (it != sections.end())
        {
            return &(*it);
        }
        else
        {
            sections.push_back({name});
            return &sections.back();
        }
    }

    void merge(const YiniDocument &other)
    {
        for (const auto &[key, value] : other.defines)
        {
            this->addDefine(key, value);
        }

        for (const auto &other_section : other.getSections())
        {
            if (other_section.name == "#include")
                continue;

            YiniSection *target_section = getOrCreateSection(other_section.name);

            if (other_section.name == "#define")
                continue;

            for (const auto &other_pair : other_section.pairs)
            {
                auto it = std::find_if(target_section->pairs.begin(), target_section->pairs.end(),
                                       [&](const YiniKeyValuePair &p) { return p.key == other_pair.key; });

                if (it != target_section->pairs.end())
                {
                    it->value = other_pair.value;
                }
                else
                {
                    target_section->pairs.push_back(other_pair);
                }
            }

            target_section->registrationList.insert(target_section->registrationList.end(),
                                                    other_section.registrationList.begin(),
                                                    other_section.registrationList.end());
        }
    }

  private:
    std::vector<YiniSection> sections;
    std::map<std::string, YiniValue> defines;
};
} // namespace YINI

#endif // YINI_DATA_HPP