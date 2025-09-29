#include "YINI/JsonDeserializer.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace YINI
{
    // Forward declaration for recursive parsing
    static bool parseJsonValue(const json& j, YiniValue& value);

    static bool parseJsonArray(const json& j_array, YiniValue& value) {
        auto yini_array = std::make_unique<YiniArray>();
        for (const auto& j_element : j_array) {
            YiniValue element_value;
            if (!parseJsonValue(j_element, element_value)) {
                return false; // Propagate failure
            }
            yini_array->elements.push_back(std::move(element_value));
        }
        value.data = std::move(yini_array);
        return true;
    }

    static bool parseJsonValue(const json& j, YiniValue& value) {
        if (j.is_string()) {
            value.data = j.get<std::string>();
        } else if (j.is_number_integer()) {
            value.data = j.get<int>();
        } else if (j.is_number_float()) {
            value.data = j.get<double>();
        } else if (j.is_boolean()) {
            value.data = j.get<bool>();
        } else if (j.is_array()) {
            return parseJsonArray(j, value);
        } else if (j.is_null()) {
            // Do nothing, default YiniValue is fine
        } else {
            return false; // Unsupported type
        }
        return true;
    }

    bool JsonDeserializer::deserialize(const std::string& json_content, YiniDocument& doc)
    {
        try
        {
            json j = json::parse(json_content);

            if (!j.is_object()) return false;

            for (auto it = j.begin(); it != j.end(); ++it)
            {
                const std::string& section_name = it.key();
                const json& section_json = it.value();

                if (!section_json.is_object()) continue;

                YiniSection* section = doc.getOrCreateSection(section_name);

                for (auto pair_it = section_json.begin(); pair_it != section_json.end(); ++pair_it)
                {
                    YiniKeyValuePair pair;
                    pair.key = pair_it.key();
                    if (!parseJsonValue(pair_it.value(), pair.value)) {
                        // Skip malformed values
                        continue;
                    }
                    section->pairs.push_back(std::move(pair));
                }
            }
        }
        catch (const json::parse_error& e)
        {
            // In a real application, log the error e.what()
            return false;
        }

        return true;
    }
}