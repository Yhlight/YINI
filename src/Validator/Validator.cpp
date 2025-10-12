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

            // 2. Validate type and range
            if (!rule.type.empty())
            {
                // `std::visit` is used to safely inspect the type held by the YiniVariant.
                // The lambda is instantiated at compile-time for each possible type in the variant.
                std::visit([&](auto&& arg) {
                    using T = std::decay_t<decltype(arg)>;
                    bool type_ok = false;

                    // `if constexpr` is used for compile-time branching. This ensures that
                    // code that would not compile for a specific type (like iterating over an int)
                    // is completely discarded, avoiding compilation errors.

                    // This branch handles the case where the variant holds a YiniArray.
                    if constexpr (std::is_same_v<T, std::unique_ptr<YiniArray>>) {
                        if (rule.type == "array") {
                            type_ok = true;
                            // If the schema specifies a subtype (e.g., "array[int]"),
                            // we must validate the type of each element in the array.
                            if (rule.array_subtype.has_value()) {
                                const std::string& subtype = rule.array_subtype.value();
                                for (const auto& item : *arg) {
                                    bool subtype_ok = false;
                                    // A nested `std::visit` is used for each element of the array.
                                    std::visit([&](auto&& item_arg) {
                                        using ItemT = std::decay_t<decltype(item_arg)>;
                                        if (subtype == "string" && std::is_same_v<ItemT, std::string>) subtype_ok = true;
                                        else if (subtype == "bool" && std::is_same_v<ItemT, bool>) subtype_ok = true;
                                        else if ((subtype == "int" || subtype == "float") && (std::is_same_v<ItemT, int64_t> || std::is_same_v<ItemT, double>)) subtype_ok = true;
                                    }, item);
                                    if (!subtype_ok) {
                                        throw std::runtime_error("Type mismatch in array for key '" + key + "'. Expected elements of type " + subtype);
                                    }
                                }
                            }
                        }
                    } else {
                        // This branch handles all non-array types (int, string, bool, etc.).
                        if (rule.type == "string" && std::is_same_v<T, std::string>) {
                            type_ok = true;
                        } else if (rule.type == "bool" && std::is_same_v<T, bool>) {
                            type_ok = true;
                        } else if ((rule.type == "int" || rule.type == "float") && (std::is_same_v<T, int64_t> || std::is_same_v<T, double>)) {
                            type_ok = true;
                            // For numeric types, also validate the range if specified in the schema.
                            double numeric_value = 0.0;
                            if constexpr (std::is_same_v<T, int64_t>) {
                                numeric_value = static_cast<double>(arg);
                            } else if constexpr (std::is_same_v<T, double>) {
                                numeric_value = arg;
                            }

                            if (rule.min && numeric_value < *rule.min) {
                                throw std::runtime_error("Value for key '" + key + "' is below the minimum of " + std::to_string(*rule.min));
                            }
                            if (rule.max && numeric_value > *rule.max) {
                                throw std::runtime_error("Value for key '" + key + "' is above the maximum of " + std::to_string(*rule.max));
                            }
                        }
                    }

                    if (!type_ok) {
                        throw std::runtime_error("Type mismatch for key '" + key + "'. Expected " + rule.type);
                    }
                }, value);
            }
        }
    }
}

}
