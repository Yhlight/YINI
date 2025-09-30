#include "YINI/YiniFormatter.hpp"
#include "YiniValueToString.hpp"
#include <sstream>

namespace YINI
{

std::string YiniFormatter::format(const YiniDocument& doc)
{
    std::stringstream ss;

    // 1. Format the [#define] block if it exists
    auto defines = doc.getDefines();
    if (!defines.empty())
    {
        ss << "[#define]\n";
        for (const auto& define_pair : defines)
        {
            ss << "  " << define_pair.first << " = " << valueToString(define_pair.second) << "\n";
        }
        ss << "\n";
    }

    // 2. Format all other sections
    auto sections = doc.getSections();
    for (const auto& section : sections)
    {
        // Skip the special define section as it's already handled
        if (section.name == "#define" || section.name == "#include") {
            continue;
        }

        // Format section header
        ss << "[" << section.name << "]";
        if (!section.inheritedSections.empty())
        {
            ss << " : ";
            for (size_t i = 0; i < section.inheritedSections.size(); ++i)
            {
                ss << section.inheritedSections[i] << (i < section.inheritedSections.size() - 1 ? ", " : "");
            }
        }
        ss << "\n";

        // Format key-value pairs
        for (const auto& pair : section.pairs)
        {
            ss << "  " << pair.key << " = " << valueToString(pair.value) << "\n";
        }

        // Format quick-registration list
        for (const auto& val : section.registrationList)
        {
            ss << "  += " << valueToString(val) << "\n";
        }

        // Add a blank line after the section for readability, unless it's the last one
        if (&section != &sections.back())
        {
            ss << "\n";
        }
    }

    return ss.str();
}

} // namespace YINI