#include "Json.h"
#include <variant>

namespace Yini
{
    // Helper to escape strings for JSON
    void writeJsonString(std::stringstream& ss, const std::string& s)
    {
        ss << '"';
        for (char c : s)
        {
            switch (c)
            {
                case '"':  ss << "\\\""; break;
                case '\\': ss << "\\\\"; break;
                case '\b': ss << "\\b"; break;
                case '\f': ss << "\\f"; break;
                case '\n': ss << "\\n"; break;
                case '\r': ss << "\\r"; break;
                case '\t': ss << "\\t"; break;
                default:   ss << c; break;
            }
        }
        ss << '"';
    }

    void JsonWriter::writeValue(std::stringstream& ss, const YiniValue& value)
    {
        std::visit([&ss](auto&& arg)
        {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::string>)
            {
                writeJsonString(ss, arg);
            }
            else if constexpr (std::is_same_v<T, int64_t> || std::is_same_v<T, double>)
            {
                ss << arg;
            }
            else if constexpr (std::is_same_v<T, bool>)
            {
                ss << (arg ? "true" : "false");
            }
            else if constexpr (std::is_same_v<T, YiniCoord>)
            {
                ss << "{\"x\":" << arg.x << ",\"y\":" << arg.y << ",\"z\":" << arg.z << ",\"is_3d\":" << (arg.is_3d ? "true" : "false") << "}";
            }
            else if constexpr (std::is_same_v<T, YiniColor>)
            {
                ss << "{\"r\":" << arg.r << ",\"g\":" << arg.g << ",\"b\":" << arg.b << "}";
            }
            else if constexpr (std::is_same_v<T, YiniObject>)
            {
                ss << '{';
                bool first = true;
                for (const auto& [key, val] : arg)
                {
                    if (!first) ss << ',';
                    writeJsonString(ss, key);
                    ss << ':';
                    writeValue(ss, val);
                    first = false;
                }
                ss << '}';
            }
            else if constexpr (std::is_same_v<T, YiniArray>)
            {
                ss << '[';
                bool first = true;
                for (const auto& val : arg)
                {
                    if (!first) ss << ',';
                    writeValue(ss, val);
                    first = false;
                }
                ss << ']';
            }
            else if constexpr (std::is_same_v<T, YiniMacroRef>)
            {
                // Should not happen as macros are resolved before this point
                ss << "\"@UNRESOLVED_MACRO_" << arg.name << "\"";
            }
        }, value.value);
    }

    std::string JsonWriter::write(const YiniValue& value)
    {
        std::stringstream ss;
        writeValue(ss, value);
        return ss.str();
    }
}
