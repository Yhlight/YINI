#include "YINI/JsonSerializer.hpp"
#include <sstream>
#include <variant>
#include <iomanip>

namespace { // Anonymous namespace for helpers

void serializeValue(std::stringstream& ss, const YINI::YiniValue& value);

void serializeArray(std::stringstream& ss, const YINI::YiniArray& array) {
    ss << "[";
    for (size_t i = 0; i < array.elements.size(); ++i) {
        serializeValue(ss, array.elements[i]);
        if (i < array.elements.size() - 1) {
            ss << ",";
        }
    }
    ss << "]";
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
        const auto& arr_ptr = std::get<std::unique_ptr<YINI::YiniArray>>(value.data);
        if (arr_ptr) serializeArray(ss, *arr_ptr);
        else ss << "null";
    } else if (std::holds_alternative<std::unique_ptr<YINI::YiniDynaValue>>(value.data)) {
        const auto& dyna_ptr = std::get<std::unique_ptr<YINI::YiniDynaValue>>(value.data);
        ss << "{\"Dyna\":";
        if (dyna_ptr) serializeValue(ss, dyna_ptr->value);
        else ss << "null";
        ss << "}";
    } else if (std::holds_alternative<std::unique_ptr<YINI::YiniCoord>>(value.data)) {
        const auto& ptr = std::get<std::unique_ptr<YINI::YiniCoord>>(value.data);
        if (ptr) ss << "{\"Coord\":{\"x\":" << ptr->x << ",\"y\":" << ptr->y << ",\"z\":" << ptr->z << ",\"is_3d\":" << (ptr->is_3d ? "true" : "false") << "}}";
        else ss << "null";
    } else if (std::holds_alternative<std::unique_ptr<YINI::YiniColor>>(value.data)) {
        const auto& ptr = std::get<std::unique_ptr<YINI::YiniColor>>(value.data);
        if (ptr) ss << "{\"Color\":{\"r\":" << (int)ptr->r << ",\"g\":" << (int)ptr->g << ",\"b\":" << (int)ptr->b << "}}";
        else ss << "null";
    } else if (std::holds_alternative<std::unique_ptr<YINI::YiniPath>>(value.data)) {
        const auto& ptr = std::get<std::unique_ptr<YINI::YiniPath>>(value.data);
        if (ptr) ss << "{\"Path\":\"" << ptr->path_value << "\"}";
        else ss << "null";
    }
}

} // end anonymous namespace

namespace YINI
{
    std::string JsonSerializer::serialize(const YiniDocument& doc)
    {
        std::stringstream ss;
        ss << "{";

        const auto& sections = doc.getSections();
        for (size_t i = 0; i < sections.size(); ++i) {
            const auto& section = sections[i];
            ss << "\"" << section.name << "\":{";

            for (size_t j = 0; j < section.pairs.size(); ++j) {
                const auto& pair = section.pairs[j];
                ss << "\"" << pair.key << "\":";
                serializeValue(ss, pair.value);
                if (j < section.pairs.size() - 1) {
                    ss << ",";
                }
            }

            ss << "}";
            if (i < sections.size() - 1) {
                ss << ",";
            }
        }

        ss << "}";
        return ss.str();
    }
}