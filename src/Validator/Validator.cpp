#include "Validator.h"
#include "YiniTypes.h"
#include <stdexcept>
#include <string>
#include <algorithm>
#include <iostream>
#include <variant>

namespace YINI
{

namespace {
    // Forward declarations for recursive validation
    void validate_value(const YiniVariant& value, const AST::SchemaRule& rule, const std::string& full_key);
    void validate_type(const YiniVariant& value, const std::string& expected_type_str, const std::string& full_key);
    void validate_range(const YiniVariant& value, const AST::SchemaRule& rule, const std::string& full_key);

    // Helper to convert a string default value from the schema into a YiniVariant
    YiniVariant convert_string_to_variant(const std::string& value_str, const std::string& type) {
        if (type == "int") {
            if (value_str.size() > 2 && value_str.substr(0, 2) == "0x") {
                return static_cast<int64_t>(std::stoll(value_str, nullptr, 16));
            }
            return static_cast<int64_t>(std::stoll(value_str));
        } else if (type == "float") {
            return std::stod(value_str);
        } else if (type == "bool") {
            return value_str == "true";
        } else if (type == "string") {
            return value_str;
        }
        // Complex types like map, array, color cannot be represented as simple string defaults in the schema yet.
        throw std::runtime_error("Default values are only supported for simple types (int, float, bool, string).");
    }

    // Parses a type string like "array[int]" into ("array", "int")
    std::pair<std::string, std::string> parse_type(const std::string& type_str) {
        auto bracket_pos = type_str.find('[');
        if (bracket_pos != std::string::npos && type_str.back() == ']') {
            std::string main_type = type_str.substr(0, bracket_pos);
            std::string sub_type = type_str.substr(bracket_pos + 1, type_str.length() - bracket_pos - 2);
            return {main_type, sub_type};
        }
        return {type_str, ""};
    }

    // Recursively validates array subtypes
    void validate_array(const YiniArray& arr, const std::string& expected_subtype_str, const std::string& full_key) {
        for (const auto& item : arr) {
            validate_type(item, expected_subtype_str, full_key);
        }
    }

    // Recursively validates list subtypes
    void validate_list(const YiniList& list, const std::string& expected_subtype_str, const std::string& full_key) {
        for (const auto& item : list.elements) {
            validate_type(item, expected_subtype_str, full_key);
        }
    }

    // Comprehensive type validation for a YiniVariant
    void validate_type(const YiniVariant& value, const std::string& expected_type_str, const std::string& full_key) {
        if (expected_type_str.empty()) return;

        auto [main_type, sub_type] = parse_type(expected_type_str);

        bool type_ok = false;
        std::visit([&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::monostate>) {
                // This shouldn't happen for a valid key, but handles it.
            } else if constexpr (std::is_same_v<T, std::string>) {
                if (main_type == "string" || main_type == "path") type_ok = true;
            } else if constexpr (std::is_same_v<T, bool>) {
                if (main_type == "bool") type_ok = true;
            } else if constexpr (std::is_same_v<T, int64_t>) {
                // An int can satisfy both int and float schema types
                if (main_type == "int" || main_type == "float") type_ok = true;
            } else if constexpr (std::is_same_v<T, double>) {
                if (main_type == "float") type_ok = true;
            } else if constexpr (std::is_same_v<T, std::unique_ptr<YiniArray>>) {
                if (main_type == "array") {
                    type_ok = true;
                    if (arg && !sub_type.empty()) {
                        validate_array(*arg, sub_type, full_key);
                    }
                }
            } else if constexpr (std::is_same_v<T, std::unique_ptr<YiniList>>) {
                if (main_type == "list") {
                    type_ok = true;
                    if (arg && !sub_type.empty()) {
                        validate_list(*arg, sub_type, full_key);
                    }
                }
            } else if constexpr (std::is_same_v<T, YiniMap>) {
                if (main_type == "map") type_ok = true;
            } else if constexpr (std::is_same_v<T, YiniStruct>) {
                if (main_type == "struct") type_ok = true;
            } else if constexpr (std::is_same_v<T, ResolvedColor>) {
                if (main_type == "color") type_ok = true;
            } else if constexpr (std::is_same_v<T, ResolvedCoord>) {
                if (main_type == "coord") type_ok = true;
            }
        }, value);

        if (!type_ok) {
            throw std::runtime_error("Type mismatch for key '" + full_key + "'. Expected type '" + expected_type_str + "'.");
        }
    }

    // Validates that a numeric value is within the min/max range
    void validate_range(const YiniVariant& value, const AST::SchemaRule& rule, const std::string& full_key) {
        if (!rule.min && !rule.max) return;

        std::visit([&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, int64_t> || std::is_same_v<T, double>) {
                double numeric_value = std::is_same_v<T, int64_t> ? static_cast<double>(arg) : arg;

                if (rule.min && numeric_value < *rule.min) {
                    throw std::runtime_error("Value for key '" + full_key + "' is below the minimum of " + std::to_string(*rule.min));
                }
                if (rule.max && numeric_value > *rule.max) {
                    throw std::runtime_error("Value for key '" + full_key + "' is above the maximum of " + std::to_string(*rule.max));
                }
            }
        }, value);
    }

    // Main validation function for a given value against a rule
    void validate_value(const YiniVariant& value, const AST::SchemaRule& rule, const std::string& full_key) {
        std::string full_type_str = rule.type;
        if ((rule.type == "array" || rule.type == "list") && !rule.array_subtype.empty()) {
            full_type_str += "[" + rule.array_subtype + "]";
        }

        validate_type(value, full_type_str, full_key);
        validate_range(value, rule, full_key);
    }
}

Validator::Validator(std::map<std::string, YiniVariant>& resolved_config, const std::vector<std::unique_ptr<AST::Stmt>>& statements)
    : m_resolved_config(resolved_config), m_statements(statements)
{
}

void Validator::validate()
{
    for (const auto& stmt : m_statements)
    {
        if (auto* schema_stmt = dynamic_cast<AST::SchemaStmt*>(stmt.get()))
        {
            for (const auto& section : schema_stmt->sections)
            {
                validate_section(section->name.lexeme, section.get());
            }
        }
    }
}

void Validator::validate_section(const std::string& section_name, const AST::SchemaSectionStmt* schema_section)
{
    for (const auto& rule_stmt : schema_section->rules)
    {
        const std::string& key = rule_stmt->key.lexeme;
        const std::string full_key = section_name + "." + key;
        const AST::SchemaRule& rule = rule_stmt->rule;
        bool key_exists = m_resolved_config.count(full_key);

        // --- Corrected Validation Logic ---

        // Step 1: Handle missing keys
        if (!key_exists) {
            // If a default value is available, apply it. This is valid for both OPTIONAL and REQUIRED keys.
            if (rule.empty_behavior == AST::SchemaRule::EmptyBehavior::ASSIGN_DEFAULT && rule.default_value.has_value()) {
                // Convert string default value to a proper variant
                YiniVariant default_val = convert_string_to_variant(rule.default_value.value(), rule.type);

                // CRITICAL FIX: Validate the default value itself against the rules
                validate_value(default_val, rule, full_key + " (default value)");

                // Add the validated default value to the configuration
                m_resolved_config[full_key] = std::move(default_val);
            }
            // If no default value, check if the key was required and error if needed.
            else if (rule.requirement == AST::SchemaRule::Requirement::REQUIRED) {
                if (rule.empty_behavior == AST::SchemaRule::EmptyBehavior::THROW_ERROR) {
                    throw std::runtime_error("Missing required key '" + full_key + "'.");
                }
                // If behavior is IGNORE, we do nothing, even if required.
            }
        }

        // Step 2: If the key now exists (either originally or via default), validate it.
        if (m_resolved_config.count(full_key)) {
            validate_value(m_resolved_config.at(full_key), rule, full_key);
        }
    }
}

}
