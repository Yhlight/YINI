#include "YINI/YiniSerializer.hpp"
#include "YiniValueToString.hpp"
#include <sstream>

namespace YINI
{

std::string YiniSerializer::serialize(const YiniDocument& document)
{
    std::stringstream ss;

    // Serialize defines
    const auto& defines = document.getDefines();
    if (!defines.empty())
    {
        ss << "[#define]\n";
        for (const auto& define_pair : defines)
        {
            ss << define_pair.first << " = " << valueToString(define_pair.second) << "\n";
        }
        ss << "\n";
    }

    // Serialize sections
    for (const auto& section : document.getSections())
    {
        ss << "[" << section.name;
        if (!section.inheritedSections.empty())
        {
            ss << " : ";
            for (size_t i = 0; i < section.inheritedSections.size(); ++i)
            {
                ss << section.inheritedSections[i] << (i < section.inheritedSections.size() - 1 ? ", " : "");
            }
        }
        ss << "]\n";

        for (const auto& pair : section.pairs)
        {
            ss << pair.key << " = " << valueToString(pair.value) << "\n";
        }

        for (const auto& val : section.registrationList)
        {
            ss << "+= " << valueToString(val) << "\n";
        }
        ss << "\n";
    }

    return ss.str();
}

} // namespace YINI