#pragma once

#include "Parser/AST.h"
#include <string>
#include <vector>
#include <map>
#include <any>
#include <optional>

namespace YINI
{

struct ValidationRule
{
    bool is_required = false;
    std::optional<std::string> type;
    std::optional<std::any> default_value;
    std::optional<double> min;
    std::optional<double> max;
    bool error_on_empty = false;
};

class Validator
{
public:
    Validator(std::map<std::string, std::any>& resolved_config, const std::vector<std::unique_ptr<AST::Stmt>>& statements);
    void validate();

private:
    void collect_schemas(const std::vector<std::unique_ptr<AST::Stmt>>& statements);
    ValidationRule parse_rule(const std::string& rule_string);
    void validate_section(const std::string& section_name, const AST::SchemaSectionStmt* schema_section);
    void validate_rule(const std::string& key, const ValidationRule& rule);

    std::map<std::string, std::any>& m_resolved_config;
    std::vector<const AST::SchemaStmt*> m_schemas;
    const std::vector<std::unique_ptr<AST::Stmt>>& m_statements;
    std::vector<std::string> m_errors;
};

} // namespace YINI