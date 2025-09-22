#include "Json.h"
#include <sstream>
#include <iomanip> // For std::quoted

namespace YINI
{

std::string Json::to_json_value(const Value& value) {
    std::stringstream ss;
    if (std::holds_alternative<std::string>(value)) {
        ss << std::quoted(std::get<std::string>(value));
    } else if (std::holds_alternative<long long>(value)) {
        ss << std::get<long long>(value);
    } else if (std::holds_alternative<double>(value)) {
        ss << std::get<double>(value);
    } else if (std::holds_alternative<bool>(value)) {
        ss << (std::get<bool>(value) ? "true" : "false");
    } else if (std::holds_alternative<std::unique_ptr<Array>>(value)) {
        ss << "[";
        const auto& arr = std::get<std::unique_ptr<Array>>(value);
        for (size_t i = 0; i < arr->elements.size(); ++i) {
            ss << to_json_value(arr->elements[i]);
            if (i < arr->elements.size() - 1) ss << ", ";
        }
        ss << "]";
    }
    // Other types...
    return ss.str();
}


std::string Json::to_json(const Document& doc)
{
    std::stringstream ss;
    ss << "{\n";

    // Defines
    ss << "  \"defines\": {\n";
    for (auto it = doc.defines.begin(); it != doc.defines.end(); ++it) {
        ss << "    " << std::quoted(it->first) << ": " << to_json_value(it->second);
        if (std::next(it) != doc.defines.end()) ss << ",";
        ss << "\n";
    }
    ss << "  },\n";

    // Includes
    ss << "  \"includes\": [\n";
    for (size_t i = 0; i < doc.includes.size(); ++i) {
        ss << "    " << std::quoted(doc.includes[i]);
        if (i < doc.includes.size() - 1) ss << ",";
        ss << "\n";
    }
    ss << "  ],\n";

    // Sections
    ss << "  \"sections\": [\n";
    for (size_t i = 0; i < doc.sections.size(); ++i) {
        const auto& sec = doc.sections[i];
        ss << "    {\n";
        ss << "      \"name\": " << std::quoted(sec.name) << ",\n";

        ss << "      \"inherits\": [";
        for (size_t j = 0; j < sec.inherited_sections.size(); ++j) {
            ss << std::quoted(sec.inherited_sections[j]) << (j < sec.inherited_sections.size() - 1 ? ", " : "");
        }
        ss << "],\n";

        ss << "      \"pairs\": {\n";
        for (auto it = sec.pairs.begin(); it != sec.pairs.end(); ++it) {
            ss << "        " << std::quoted(it->key) << ": " << to_json_value(it->value);
            if (std::next(it) != sec.pairs.end()) ss << ",";
            ss << "\n";
        }
        ss << "      },\n";

        ss << "      \"anonymous_values\": [";
        for (size_t j = 0; j < sec.anonymous_values.size(); ++j) {
            ss << to_json_value(sec.anonymous_values[j]);
            if (j < sec.anonymous_values.size() - 1) ss << ", ";
        }
        ss << "]\n";

        ss << "    }" << (i < doc.sections.size() - 1 ? "," : "") << "\n";
    }
    ss << "  ]\n";

    ss << "}\n";
    return ss.str();
}

} // namespace YINI
