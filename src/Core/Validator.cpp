#include "Validator.h"
#include "Core/YiniException.h"
#include <algorithm>
#include <sstream>

namespace YINI
{
    // Private helper to parse a validation rule string like "string, required"
    static std::pair<std::string, bool> parse_rule(const std::string& rule_str) {
        std::stringstream ss(rule_str);
        std::string type;
        std::string attribute;
        bool required = false;

        std::getline(ss, type, ',');
        type.erase(0, type.find_first_not_of(" \t\n\r"));
        type.erase(type.find_last_not_of(" \t\n\r") + 1);

        if (std::getline(ss, attribute)) {
            attribute.erase(0, attribute.find_first_not_of(" \t\n\r"));
            attribute.erase(attribute.find_last_not_of(" \t\n\r") + 1);
            if (attribute == "required") {
                required = true;
            }
        }
        return {type, required};
    }

    void Validator::validate_type(const std::string& type_str, const YiniValue& value, const std::string& full_key, std::vector<ValidationError>& errors) {
        if (type_str == "string" && !std::holds_alternative<std::string>(value.m_value)) {
            errors.push_back({ "Type mismatch for '" + full_key + "': expected string." });
        } else if (type_str == "number" && !std::holds_alternative<double>(value.m_value)) {
            errors.push_back({ "Type mismatch for '" + full_key + "': expected number." });
        } else if (type_str == "bool" && !std::holds_alternative<bool>(value.m_value)) {
            errors.push_back({ "Type mismatch for '" + full_key + "': expected bool." });
        } else if (type_str == "array" && !std::holds_alternative<std::unique_ptr<YiniArray>>(value.m_value)) {
            errors.push_back({ "Type mismatch for '" + full_key + "': expected array." });
        } else if (type_str.rfind("array[", 0) == 0) { // starts with array[
            if (!std::holds_alternative<std::unique_ptr<YiniArray>>(value.m_value)) {
                errors.push_back({ "Type mismatch for '" + full_key + "': expected array." });
                return;
            }
            std::string subtype = type_str.substr(6, type_str.length() - 7);
            const auto& arr = *std::get<std::unique_ptr<YiniArray>>(value.m_value);
            for(const auto& elem : arr) {
                validate_type(subtype, elem, full_key + "[]", errors);
            }
        } else if (type_str == "map" && !std::holds_alternative<std::unique_ptr<YiniMap>>(value.m_value)) {
            errors.push_back({ "Type mismatch for '" + full_key + "': expected map." });
        }
    }

    std::vector<ValidationError> Validator::validate(const Schema& schema, const Interpreter& interpreter)
    {
        std::vector<ValidationError> errors;

        for (const auto& schema_section : schema.sections) {
            const std::string& section_name = schema_section->name.lexeme;

            // Check if the section exists in the data
            if (interpreter.resolved_sections.find(section_name) == interpreter.resolved_sections.end()) {
                // Before failing, check if all keys are optional
                bool all_optional = true;
                for(const auto& stmt : schema_section->statements) {
                    if (auto* kv = dynamic_cast<const KeyValue*>(stmt.get())) {
                        auto* rule_literal = dynamic_cast<const Literal*>(kv->value.get());
                        if (rule_literal && std::holds_alternative<std::string>(rule_literal->value.m_value)) {
                            auto rule = parse_rule(std::get<std::string>(rule_literal->value.m_value));
                            if (rule.second) { // if required
                                all_optional = false;
                                break;
                            }
                        }
                    }
                }
                if (!all_optional) {
                    errors.push_back({ "Required section '" + section_name + "' is missing." });
                }
                continue; // Continue to next schema section
            }

            const auto& data_section = interpreter.resolved_sections.at(section_name);

            // Iterate through keys in the schema
            for (const auto& stmt : schema_section->statements) {
                if (auto* kv = dynamic_cast<const KeyValue*>(stmt.get())) {
                    const std::string& key_name = kv->key.lexeme;
                    std::string full_key = section_name + "." + key_name;

                    auto* rule_literal = dynamic_cast<const Literal*>(kv->value.get());
                    if (!rule_literal || !std::holds_alternative<std::string>(rule_literal->value.m_value)) {
                        errors.push_back({ "Invalid schema rule for '" + full_key + "'." });
                        continue;
                    }

                    auto rule = parse_rule(std::get<std::string>(rule_literal->value.m_value));
                    std::string type = rule.first;
                    bool required = rule.second;

                    // Check if key exists in data
                    if (data_section.find(key_name) == data_section.end()) {
                        if (required) {
                            errors.push_back({ "Required key '" + full_key + "' is missing." });
                        }
                    } else {
                        // If key exists, validate its type
                        const auto& value = data_section.at(key_name);
                        validate_type(type, value, full_key, errors);
                    }
                }
            }
        }

        return errors;
    }
}