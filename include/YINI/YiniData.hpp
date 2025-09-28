#ifndef YINI_DATA_HPP
#define YINI_DATA_HPP

#include <string>
#include <vector>
#include <variant>
#include <map>
#include <memory>

namespace YINI
{
    // Forward declarations
    struct YiniValue;
    struct YiniArray;
    struct YiniMap;

    // YiniVariant holds the possible types. Recursive types are held by unique_ptr.
    using YiniVariant = std::variant<
        std::string,
        int,
        double,
        bool,
        std::unique_ptr<YiniArray>,
        std::unique_ptr<YiniMap>
    >;

    // YiniValue is a wrapper around the variant to enable recursion.
    struct YiniValue
    {
        YiniVariant data;

        // Constructors
        YiniValue();
        YiniValue(const YiniValue& other);
        YiniValue(YiniValue&& other) noexcept;

        // Assignment operators
        YiniValue& operator=(const YiniValue& other);
        YiniValue& operator=(YiniValue&& other) noexcept;

        // Destructor
        ~YiniValue();
    };

    // YiniArray contains a vector of YiniValues.
    struct YiniArray {
        std::vector<YiniValue> elements;
    };

    // YiniMap contains a map of string to YiniValue.
    struct YiniMap {
        std::map<std::string, YiniValue> elements;
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
        void addSection(const YiniSection& section)
        {
            sections.push_back(section);
        }

        void addSection(YiniSection&& section)
        {
            sections.push_back(std::move(section));
        }

        const std::vector<YiniSection>& getSections() const
        {
            return sections;
        }

    private:
        std::vector<YiniSection> sections;
        std::map<std::string, YiniValue> defines;
    };
}

#endif // YINI_DATA_HPP