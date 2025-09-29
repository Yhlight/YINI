#include "YINI/YiniSerializer.hpp"
#include <sstream>
#include <variant>

namespace { // Anonymous namespace for helpers

void serializeValue(std::stringstream& ss, const YINI::YiniValue& value);

void serializeArray(std::stringstream& ss, const YINI::YiniArray& array) {
    ss << "[";
    for (size_t i = 0; i < array.elements.size(); ++i) {
        serializeValue(ss, array.elements[i]);
        if (i < array.elements.size() - 1) {
            ss << ", ";
        }
    }
    ss << "]";
}

void serializeMap(std::stringstream& ss, const YINI::YiniMap& map) {
    ss << "{{";
    for (auto it = map.elements.cbegin(); it != map.elements.cend(); ) {
        ss << it->first << ": ";
        serializeValue(ss, it->second);
        if (++it != map.elements.cend()) {
            ss << ", ";
        }
    }
    ss << "}}";
}

void serializePair(std::stringstream& ss, const YINI::YiniPair& pair) {
    ss << "{" << pair.key << ": ";
    serializeValue(ss, pair.value);
    ss << "}";
}

void serializeValue(std::stringstream& ss, const YINI::YiniValue& value) {
    if (std::holds_alternative<std::string>(value.data)) {
        ss << "\"" << std::get<std::string>(value.data) << "\"";
    } else if (std::holds_alternative<int>(value.data)) {
        ss << std::get<int>(value.data);
    } else if (std::holds_alternative<double>(value.data)) {
        double d = std::get<double>(value.data);
        ss << d;
        if (d == static_cast<long long>(d)) {
            ss << ".0";
        }
    } else if (std::holds_alternative<bool>(value.data)) {
        ss << (std::get<bool>(value.data) ? "true" : "false");
    } else if (std::holds_alternative<std::unique_ptr<YINI::YiniArray>>(value.data)) {
        serializeArray(ss, *std::get<std::unique_ptr<YINI::YiniArray>>(value.data));
    } else if (std::holds_alternative<std::unique_ptr<YINI::YiniMap>>(value.data)) {
        serializeMap(ss, *std::get<std::unique_ptr<YINI::YiniMap>>(value.data));
    } else if (std::holds_alternative<std::unique_ptr<YINI::YiniPair>>(value.data)) {
        serializePair(ss, *std::get<std::unique_ptr<YINI::YiniPair>>(value.data));
    } else if (std::holds_alternative<std::unique_ptr<YINI::YiniDynaValue>>(value.data)) {
        ss << "Dyna(";
        serializeValue(ss, std::get<std::unique_ptr<YINI::YiniDynaValue>>(value.data)->value);
        ss << ")";
    } else if (std::holds_alternative<std::unique_ptr<YINI::YiniCoord>>(value.data)) {
        const auto& coord = *std::get<std::unique_ptr<YINI::YiniCoord>>(value.data);
        ss << "Coord(" << coord.x << ", " << coord.y;
        if (coord.is_3d) {
            ss << ", " << coord.z;
        }
        ss << ")";
    } else if (std::holds_alternative<std::unique_ptr<YINI::YiniColor>>(value.data)) {
        const auto& color = *std::get<std::unique_ptr<YINI::YiniColor>>(value.data);
        ss << "Color(" << (int)color.r << ", " << (int)color.g << ", " << (int)color.b << ")";
    } else if (std::holds_alternative<std::unique_ptr<YINI::YiniPath>>(value.data)) {
        ss << "Path(" << std::get<std::unique_ptr<YINI::YiniPath>>(value.data)->path_value << ")";
    }
}

} // end anonymous namespace

namespace YINI
{
    std::string YiniSerializer::serialize(const YiniDocument& doc)
    {
        std::stringstream ss;

        // Handle defines first, if they exist
        const auto& defines = doc.getDefines();
        if (!defines.empty())
        {
            ss << "[#define]\n";
            for (const auto& [key, value] : defines)
            {
                ss << key << " = ";
                serializeValue(ss, value);
                ss << "\n";
            }
            ss << "\n";
        }

        const auto& sections = doc.getSections();
        for (const auto& section : sections) {
            if(section.name == "#define" || section.name == "#include") continue;

            ss << "[" << section.name;
            if (!section.inheritedSections.empty()) {
                ss << " : ";
                for (size_t i = 0; i < section.inheritedSections.size(); ++i) {
                    ss << section.inheritedSections[i];
                    if (i < section.inheritedSections.size() - 1) {
                        ss << ", ";
                    }
                }
            }
            ss << "]\n";

            for (const auto& pair : section.pairs) {
                ss << pair.key << " = ";
                serializeValue(ss, pair.value);
                ss << "\n";
            }

            for (const auto& value : section.registrationList) {
                ss << "+= ";
                serializeValue(ss, value);
                ss << "\n";
            }
            ss << "\n";
        }
        return ss.str();
    }
}