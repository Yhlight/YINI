#include "Validator.h"
#include <stdexcept>
#include <string>
#include <algorithm>
#include <iostream>
#include <variant>

namespace YINI
{

namespace {
    YiniVariant convert_string_to_variant(const std::string& value_str, const std::string& type) {
        if (type == "int") {
            // Check for C-style hex literal
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
        return std::monostate{};
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

    // Forward declaration for recursion
    void validate_variant(const YiniVariant& value, const std::string& expected_type_str, const std::string& full_key);

    void validate_array(const YiniArray& arr, const std::string& expected_subtype_str, const std::string& full_key) {
        for (const auto& item : arr) {
            validate_variant(item, expected_subtype_str, full_key);
        }
    }

    void validate_variant(const YiniVariant& value, const std::string& expected_type_str, const std::string& full_key) {
        auto [main_type, sub_type] = parse_type(expected_type_str);

        bool type_ok = false;
        std::visit([&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, std::string>) {
                if (main_type == "string") type_ok = true;
            }
            else if constexpr (std::is_same_v<T, bool>) {
                if (main_type == "bool") type_ok = true;
            }
            else if constexpr (std::is_same_v<T, int64_t>) {
                if (main_type == "int" || main_type == "float") type_ok = true;
            }
            else if constexpr (std::is_same_v<T, double>) {
                if (main_type == "float") type_ok = true;
            }
            else if constexpr (std::is_same_v<T, std::unique_ptr<YiniArray>>) {
                if (main_type == "array") {
                    type_ok = true;
                    if (!sub_type.empty()) {
                        validate_array(*arg, sub_type, full_key);
                    }
                }
            }
        }, value);

        if (!type_ok) {
            throw std::runtime_error("Type mismatch for key '" + full_key + "'. Expected " + expected_type_str);
        }
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
        std::string key = rule_stmt->key.lexeme;
        std::string full_key = section_name + "." + key;
        const AST::SchemaRule& rule = rule_stmt->rule;
        bool key_exists = m_resolved_config.count(full_key);

        // 1. Check for required keys
        if (rule.requirement == AST::SchemaRule::Requirement::REQUIRED && !key_exists)
        {
            if (rule.empty_behavior == AST::SchemaRule::EmptyBehavior::ASSIGN_DEFAULT && rule.default_value.has_value())
            {
                m_resolved_config[full_key] = convert_string_to_variant(rule.default_value.value(), rule.type);
            }
            else if (rule.empty_behavior == AST::SchemaRule::EmptyBehavior::THROW_ERROR)
            {
                throw std::runtime_error("Missing required key '" + key + "' in section '" + section_name + "'.");
            }
        }

        if (key_exists)
        {
            YiniVariant& value = m_resolved_config.at(full_key);

            // 2. Validate type
            if (!rule.type.empty())
            {
                std::string full_type_str = rule.type;
                if (rule.type == "array" && !rule.array_subtype.empty()) {
                    full_type_str = "array[" + rule.array_subtype + "]";
                }
                validate_variant(value, full_type_str, full_key);
            }

            // 3. Validate range
            std::visit([&](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, int64_t> || std::is_same_v<T, double>)
                {
                    double numeric_value = 0.0;
                    if constexpr (std::is_same_v<T, int64_t>) numeric_value = static_cast<double>(arg);
                    if constexpr (std::is_same_v<T, double>) numeric_value = arg;

                    if (rule.min && numeric_value < *rule.min) {
                        throw std::runtime_error("Value for key '" + key + "' is below the minimum of " + std::to_string(*rule.min));
                    }
                    if (rule.max && numeric_value > *rule.max) {
                        throw std::runtime_error("Value for key '" + key + "' is above the maximum of " + std::to_string(*rule.max));
                    }
                }
            }, value);
        }
    }
}

}
