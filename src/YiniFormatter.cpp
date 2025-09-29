#include "YINI/YiniFormatter.hpp"
#include <sstream>
#include <iomanip> // For std::quoted

namespace YINI
{
    // Forward declaration for recursive formatting
    static void formatValue(std::stringstream& ss, const YiniValue& value);

    static void formatArray(std::stringstream& ss, const YiniArray& array)
    {
        ss << "[";
        for (size_t i = 0; i < array.elements.size(); ++i)
        {
            formatValue(ss, array.elements[i]);
            if (i < array.elements.size() - 1)
            {
                ss << ", ";
            }
        }
        ss << "]";
    }

    static void formatList(std::stringstream& ss, const YiniList& list)
    {
        ss << "List(";
        for (size_t i = 0; i < list.elements.size(); ++i)
        {
            formatValue(ss, list.elements[i]);
            if (i < list.elements.size() - 1)
            {
                ss << ", ";
            }
        }
        ss << ")";
    }

    static void formatSet(std::stringstream& ss, const YiniSet& set)
    {
        ss << "Set(";
        size_t count = 0;
        for (const auto& elem : set.elements)
        {
            formatValue(ss, elem);
            if (count < set.elements.size() - 1)
            {
                ss << ", ";
            }
            count++;
        }
        ss << ")";
    }

    static void formatMap(std::stringstream& ss, const YiniMap& map)
    {
        ss << "{";
        size_t count = 0;
        for (const auto& [key, value] : map.elements)
        {
            ss << key << ": ";
            formatValue(ss, value);
            if (count < map.elements.size() - 1)
            {
                ss << ", ";
            }
            count++;
        }
        ss << "}";
    }

    static void formatTuple(std::stringstream& ss, const YiniTuple& tuple)
    {
        ss << "{" << tuple.key << ": ";
        formatValue(ss, tuple.value);
        ss << "}";
    }

    static void formatCoord(std::stringstream& ss, const YiniCoord& coord)
    {
        ss << "Coord(" << coord.x << ", " << coord.y;
        if (coord.is_3d)
        {
            ss << ", " << coord.z;
        }
        ss << ")";
    }

    static void formatColor(std::stringstream& ss, const YiniColor& color)
    {
        ss << "Color(" << (int)color.r << ", " << (int)color.g << ", " << (int)color.b << ")";
    }

    static void formatPath(std::stringstream& ss, const YiniPath& path)
    {
        ss << "Path(" << path.path_value << ")";
    }

    static void formatValue(std::stringstream& ss, const YiniValue& value)
    {
        std::visit([&ss](const auto& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::string>)
                ss << std::quoted(arg);
            else if constexpr (std::is_same_v<T, int> || std::is_same_v<T, double>)
                ss << arg;
            else if constexpr (std::is_same_v<T, bool>)
                ss << (arg ? "true" : "false");
            else if constexpr (std::is_same_v<T, std::unique_ptr<YiniArray>>)
                if(arg) formatArray(ss, *arg); else ss << "[]";
            else if constexpr (std::is_same_v<T, std::unique_ptr<YiniList>>)
                if(arg) formatList(ss, *arg); else ss << "List()";
            else if constexpr (std::is_same_v<T, std::unique_ptr<YiniSet>>)
                if(arg) formatSet(ss, *arg); else ss << "Set()";
            else if constexpr (std::is_same_v<T, std::unique_ptr<YiniTuple>>)
                if(arg) formatTuple(ss, *arg); else ss << "{}";
            else if constexpr (std::is_same_v<T, std::unique_ptr<YiniMap>>)
                if(arg) formatMap(ss, *arg); else ss << "{}";
            else if constexpr (std::is_same_v<T, std::unique_ptr<YiniCoord>>)
                if(arg) formatCoord(ss, *arg); else ss << "Coord(0,0)";
            else if constexpr (std::is_same_v<T, std::unique_ptr<YiniColor>>)
                if(arg) formatColor(ss, *arg); else ss << "Color(0,0,0)";
            else if constexpr (std::is_same_v<T, std::unique_ptr<YiniPath>>)
                if(arg) formatPath(ss, *arg); else ss << "Path()";
            // Note: YiniDynaValue is unwrapped during parsing, so it's not handled here.
        }, value.data);
    }

    std::string YiniFormatter::format(const YiniValue& value)
    {
        std::stringstream ss;
        formatValue(ss, value);
        return ss.str();
    }

    std::string YiniFormatter::formatDocument(const YiniDocument& doc)
    {
        std::stringstream ss;

        // Format defines
        const auto& defines = doc.getDefines();
        if (!defines.empty())
        {
            ss << "[#define]\n";
            for (const auto& [key, define] : defines)
            {
                ss << key << " = " << format(define.value) << "\n";
            }
            ss << "\n";
        }

        // Format sections
        for (const auto& section : doc.getSections())
        {
            if (section.name.empty() || section.name[0] == '#') continue;

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
                ss << pair.key << " = " << (pair.is_dynamic ? "Dyna(" : "") << format(pair.value) << (pair.is_dynamic ? ")" : "") << "\n";
            }

            for (const auto& reg_val : section.registrationList)
            {
                ss << "+= " << format(reg_val) << "\n";
            }
            ss << "\n";
        }

        return ss.str();
    }
}