#include "Validator.h"
#include <stdexcept>
#include <string>
#include <algorithm>
#include <iostream>

namespace YINI
{

Validator::Validator(std::map<std::string, std::any>& resolved_config, const std::vector<std::unique_ptr<AST::Stmt>>& statements)
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
            if (rule.empty_behavior == AST::SchemaRule::EmptyBehavior::ASSIGN_DEFAULT && rule.default_value)
            {
                // Apply default value
                // This part needs to be improved to handle different types
                m_resolved_config[full_key] = std::stod(*rule.default_value);
            }
            else if (rule.empty_behavior == AST::SchemaRule::EmptyBehavior::THROW_ERROR)
            {
                throw std::runtime_error("Missing required key '" + key + "' in section '" + section_name + "'.");
            }
            // If IGNORE, do nothing.
        }

        if (key_exists)
        {
            std::any& value = m_resolved_config.at(full_key);

            std::cout << "Validating key '" << full_key << "'. Rule type: '" << rule.type << "'. Actual value type: '" << value.type().name() << "'" << std::endl;

            // 2. Validate type
            if (!rule.type.empty())
            {
                bool is_numeric_rule = rule.type == "int" || rule.type == "float";
                bool is_string_rule = rule.type == "string";

                bool is_value_numeric = value.type() == typeid(double);
                bool is_value_string = value.type() == typeid(std::string);

                if (is_string_rule && !is_value_string)
                {
                    throw std::runtime_error("Type mismatch for key '" + key + "'. Expected string.");
                }

                if (is_numeric_rule && !is_value_numeric)
                {
                    throw std::runtime_error("Type mismatch for key '" + key + "'. Expected number.");
                }
            }

            // 3. Validate range
            if (value.type() == typeid(double))
            {
                double numeric_value = std::any_cast<double>(value);
                if (rule.min && numeric_value < *rule.min)
                {
                    throw std::runtime_error("Value for key '" + key + "' is below the minimum of " + std::to_string(*rule.min));
                }
                if (rule.max && numeric_value > *rule.max)
                {
                    throw std::runtime_error("Value for key '" + key + "' is above the maximum of " + std::to_string(*rule.max));
                }
            }
        }
    }
}

}
