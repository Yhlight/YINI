#include "Validator.h"
#include "YiniTypes.h"
#include <sstream>
#include <iostream>

namespace YINI
{

Validator::Validator(std::map<std::string, std::any>& resolved_config, const std::vector<std::unique_ptr<AST::Stmt>>& statements)
    : m_resolved_config(resolved_config)
{
    collect_schemas(statements);
}

void Validator::validate()
{
    for (const auto* schema : m_schemas)
    {
        for (const auto& section : schema->sections)
        {
            validate_section(section->name.lexeme, section.get());
        }
    }

    if (!m_errors.empty())
    {
        std::stringstream error_stream;
        error_stream << "Schema validation failed with " << m_errors.size() << " errors:\n";
        for (const auto& error : m_errors)
        {
            error_stream << "- " << error << "\n";
        }
        throw std::runtime_error(error_stream.str());
    }
}

void Validator::collect_schemas(const std::vector<std::unique_ptr<AST::Stmt>>& statements)
{
    for (const auto& stmt : statements)
    {
        if (auto* schema_stmt = dynamic_cast<const AST::SchemaStmt*>(stmt.get()))
        {
            m_schemas.push_back(schema_stmt);
        }
    }
}

void Validator::validate_section(const std::string& section_name, const AST::SchemaSectionStmt* schema_section)
{
    for (const auto& rule_stmt : schema_section->rules)
    {
        ValidationRule rule = parse_rule(rule_stmt->rules.lexeme);
        validate_rule(section_name + "." + rule_stmt->key.lexeme, rule);
    }
}

ValidationRule Validator::parse_rule(const std::string& rule_string) {
    ValidationRule rule;
    std::stringstream ss(rule_string);
    std::string segment;

    while (std::getline(ss, segment, ',')) {
        // Trim leading/trailing whitespace
        segment.erase(0, segment.find_first_not_of(" \t\n\r\f\v"));
        segment.erase(segment.find_last_not_of(" \t\n\r\f\v") + 1);

        if (segment == "!") {
            rule.is_required = true;
        } else if (segment == "?") {
            rule.is_required = false;
        } else if (segment == "e") {
            rule.error_on_empty = true;
        } else if (segment == "~") {
            // ignore on empty, do nothing
        } else if (segment.rfind("=", 0) == 0) {
            std::string value_str = segment.substr(1);
            try {
                rule.default_value = std::stod(value_str);
            } catch (...) {
                if (value_str == "true") rule.default_value = true;
                else if (value_str == "false") rule.default_value = false;
                else rule.default_value = value_str;
            }
        } else if (segment.rfind("min=", 0) == 0) {
            rule.min = std::stod(segment.substr(4));
        } else if (segment.rfind("max=", 0) == 0) {
            rule.max = std::stod(segment.substr(4));
        } else {
            size_t start_bracket = segment.find('[');
            if (start_bracket != std::string::npos) {
                size_t end_bracket = segment.find(']');
                if (end_bracket != std::string::npos) {
                    rule.type = segment.substr(0, start_bracket);
                    rule.inner_type = segment.substr(start_bracket + 1, end_bracket - start_bracket - 1);
                } else {
                    rule.type = segment;
                }
            } else {
                rule.type = segment;
            }
        }
    }
    return rule;
}

void Validator::validate_rule(const std::string& key, const ValidationRule& rule)
{
    // Step 1: Handle missing keys and apply defaults
    if (!m_resolved_config.count(key))
    {
        if (rule.is_required)
        {
            if (rule.default_value.has_value())
            {
                m_resolved_config[key] = rule.default_value.value();
            }
            else
            {
                m_errors.push_back("Required key '" + key + "' is missing.");
                return; // No value to validate
            }
        }
        else
        {
            if (rule.error_on_empty)
            {
                m_errors.push_back("Optional key '" + key + "' with 'error on empty' rule is missing.");
            }
            return; // Optional key is missing, nothing to do unless 'e' is specified.
        }
    }

    // Now the key is guaranteed to exist.
    const auto& value = m_resolved_config.at(key);

    // Step 2: Type validation
    if (rule.type.has_value())
    {
        const std::string& type_str = rule.type.value();
        bool type_ok = false;
        if ((type_str == "int" || type_str == "float") && value.type() == typeid(double)) {
            type_ok = true;
        } else if ((type_str == "string" || type_str == "path") && value.type() == typeid(std::string)) {
            type_ok = true;
        } else if (type_str == "bool" && value.type() == typeid(bool)) {
            type_ok = true;
        } else if ((type_str == "array" || type_str == "list" || type_str == "set") && value.type() == typeid(std::vector<std::any>)) {
            type_ok = true;
        } else if (type_str == "map" && value.type() == typeid(std::map<std::string, std::any>)) {
            type_ok = true;
        } else if (type_str == "color" && value.type() == typeid(ResolvedColor)) {
            type_ok = true;
        } else if (type_str == "coord" && value.type() == typeid(ResolvedCoord)) {
            type_ok = true;
        }

        if (!type_ok) {
            m_errors.push_back("Key '" + key + "' has incorrect type. Expected " + type_str + " but found " + value.type().name() + ".");
            return; // Stop validation if type is wrong
        }

        // Step 2b: Inner type validation for containers
        if (rule.inner_type.has_value() && value.type() == typeid(std::vector<std::any>)) {
            const auto& container = std::any_cast<const std::vector<std::any>&>(value);
            const std::string& inner_type_str = rule.inner_type.value();
            for (const auto& element : container) {
                bool inner_type_ok = false;
                if ((inner_type_str == "int" || inner_type_str == "float") && element.type() == typeid(double)) {
                    inner_type_ok = true;
                } else if (inner_type_str == "string" && element.type() == typeid(std::string)) {
                    inner_type_ok = true;
                } else if (inner_type_str == "bool" && element.type() == typeid(bool)) {
                    inner_type_ok = true;
                }
                // ... can be extended for nested containers etc.

                if (!inner_type_ok) {
                    m_errors.push_back("Key '" + key + "' has element with incorrect type. Expected " + inner_type_str + ".");
                    // Don't return, just report all invalid elements
                }
            }
        }
    }

    // Step 3: Range validation
    if (value.type() == typeid(double))
    {
        double numeric_value = std::any_cast<double>(value);
        if (rule.min.has_value() && numeric_value < rule.min.value())
        {
            m_errors.push_back("Key '" + key + "' value " + std::to_string(numeric_value) + " is less than min " + std::to_string(rule.min.value()) + ".");
        }
        if (rule.max.has_value() && numeric_value > rule.max.value())
        {
            m_errors.push_back("Key '" + key + "' value " + std::to_string(numeric_value) + " is greater than max " + std::to_string(rule.max.value()) + ".");
        }
    }
}

} // namespace YINI