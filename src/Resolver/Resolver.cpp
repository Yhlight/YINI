#include "Resolver.h"
#include <stdexcept>
#include <cmath>
#include <fstream>
#include <sstream>
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include <cstdlib>

namespace YINI
{

Resolver::Resolver(const std::vector<std::unique_ptr<AST::Stmt>>& statements, YmetaManager& ymeta_manager)
    : m_statements(statements), m_ymeta_manager(ymeta_manager)
{
}

Resolver::ResolvedConfig Resolver::resolve()
{
    // First pass: collect all section definitions
    for (const auto& stmt : m_statements)
    {
        if (auto* section_stmt = dynamic_cast<AST::SectionStmt*>(stmt.get()))
        {
            m_sections[section_stmt->name.lexeme] = section_stmt;
        }
    }

    // Second pass: resolve all statements
    for (const auto& stmt : m_statements)
    {
        resolve_statement(stmt.get());
    }
    return m_resolved_config;
}

void Resolver::resolve_statement(AST::Stmt* stmt)
{
    if (auto* define_stmt = dynamic_cast<AST::DefineSectionStmt*>(stmt))
    {
        visit_define_section(define_stmt);
    }
    else if (auto* section_stmt = dynamic_cast<AST::SectionStmt*>(stmt))
    {
        visit_section(section_stmt);
    }
    else if (auto* include_stmt = dynamic_cast<AST::IncludeStmt*>(stmt))
    {
        visit_include(include_stmt);
    }
    else if (auto* kv_stmt = dynamic_cast<AST::KeyValueStmt*>(stmt))
    {
        visit_key_value(kv_stmt);
    }
    else if (auto* qr_stmt = dynamic_cast<AST::QuickRegStmt*>(stmt))
    {
        visit_quick_reg(qr_stmt);
    }
}

std::any Resolver::resolve_expression(AST::Expr* expr)
{
    if (auto* literal_expr = dynamic_cast<AST::LiteralExpr*>(expr))
    {
        return visit_literal(literal_expr);
    }
    else if (auto* bool_expr = dynamic_cast<AST::BoolExpr*>(expr))
    {
        return visit_bool(bool_expr);
    }
    else if (auto* array_expr = dynamic_cast<AST::ArrayExpr*>(expr))
    {
        return visit_array(array_expr);
    }
    else if (auto* set_expr = dynamic_cast<AST::SetExpr*>(expr))
    {
        return visit_set(set_expr);
    }
    else if (auto* map_expr = dynamic_cast<AST::MapExpr*>(expr))
    {
        return visit_map(map_expr);
    }
    else if (auto* color_expr = dynamic_cast<AST::ColorExpr*>(expr))
    {
        return visit_color(color_expr);
    }
    else if (auto* coord_expr = dynamic_cast<AST::CoordExpr*>(expr))
    {
        return visit_coord(coord_expr);
    }
    else if (auto* macro_expr = dynamic_cast<AST::MacroExpr*>(expr))
    {
        return visit_macro(macro_expr);
    }
    else if (auto* binary_expr = dynamic_cast<AST::BinaryExpr*>(expr))
    {
        return visit_binary(binary_expr);
    }
    else if (auto* grouping_expr = dynamic_cast<AST::GroupingExpr*>(expr))
    {
        return visit_grouping(grouping_expr);
    }
    else if (auto* cs_ref_expr = dynamic_cast<AST::CrossSectionRefExpr*>(expr))
    {
        return visit_cross_section_ref(cs_ref_expr);
    }
    else if (auto* env_var_ref_expr = dynamic_cast<AST::EnvVarRefExpr*>(expr))
    {
        return visit_env_var_ref(env_var_ref_expr);
    }
    else if (auto* path_expr = dynamic_cast<AST::PathExpr*>(expr))
    {
        return visit_path(path_expr);
    }
    else if (auto* list_expr = dynamic_cast<AST::ListExpr*>(expr))
    {
        return visit_list(list_expr);
    }
    throw std::runtime_error("Unknown expression type.");
}

void Resolver::visit_define_section(AST::DefineSectionStmt* stmt)
{
    for (const auto& definition : stmt->definitions)
    {
        m_macros[definition->key.lexeme] = definition->value.get();
    }
}

void Resolver::visit_section(AST::SectionStmt* stmt)
{
    if (m_resolved_sections.count(stmt->name.lexeme)) {
        return; // Already resolved
    }

    if (m_resolving_stack.count(stmt->name.lexeme)) {
        throw std::runtime_error("Circular inheritance detected for section: " + stmt->name.lexeme);
    }

    m_resolving_stack.insert(stmt->name.lexeme);

    for (const auto& parent_name_token : stmt->parent_sections)
    {
        std::string parent_name = parent_name_token.lexeme;
        if (m_sections.count(parent_name))
        {
            visit_section(m_sections[parent_name]);
            // Copy resolved values from parent
            if(m_resolved_config.count(parent_name)) {
                for (auto const& [key, val] : m_resolved_config.at(parent_name)) {
                    m_resolved_config[stmt->name.lexeme][key] = val;
                }
            }
        }
        else
        {
            throw std::runtime_error("Undefined parent section: " + parent_name);
        }
    }

    m_current_section = stmt->name.lexeme;
    for (const auto& statement : stmt->statements)
    {
        resolve_statement(statement.get());
    }
    m_current_section.clear();

    m_resolving_stack.erase(stmt->name.lexeme);
    m_resolved_sections.insert(stmt->name.lexeme);
}

void Resolver::visit_include(AST::IncludeStmt* stmt)
{
    for (const auto& path_expr : stmt->paths)
    {
        std::any path_any = resolve_expression(path_expr.get());
        if (path_any.type() != typeid(std::string))
        {
            throw std::runtime_error("Include path must be a string.");
        }
        std::string path = std::any_cast<std::string>(path_any);

        std::ifstream file(path);
        if (!file.is_open())
        {
            throw std::runtime_error("Could not open included file: " + path);
        }

        std::stringstream buffer;
        buffer << file.rdbuf();

        Lexer lexer(buffer.str());
        auto tokens = lexer.scan_tokens();
        Parser parser(tokens);
        auto included_ast = parser.parse();

        for (const auto& included_stmt : included_ast)
        {
            resolve_statement(included_stmt.get());
        }
    }
}

void Resolver::visit_key_value(AST::KeyValueStmt* stmt)
{
    std::string section_name = m_current_section.empty() ? "" : m_current_section;
    std::string key = stmt->key.lexeme;
    std::string full_key = section_name.empty() ? key : section_name + "." + key;

    if (auto* dyna_expr = dynamic_cast<AST::DynaExpr*>(stmt->value.get()))
    {
        if (m_ymeta_manager.has_value(full_key))
        {
            m_resolved_config[section_name][key] = m_ymeta_manager.get_value(full_key);
        }
        else
        {
            std::any value = resolve_expression(dyna_expr->expression.get());
            m_ymeta_manager.set_value(full_key, value);
            m_resolved_config[section_name][key] = value;
        }
    }
    else
    {
        m_resolved_config[section_name][key] = resolve_expression(stmt->value.get());
    }
}

std::any Resolver::visit_literal(AST::LiteralExpr* expr)
{
    if (std::holds_alternative<std::string>(expr->value.literal))
    {
        return std::get<std::string>(expr->value.literal);
    }
    else if (std::holds_alternative<double>(expr->value.literal))
    {
        return std::get<double>(expr->value.literal);
    }
    return {}; // Should not be reached
}

std::any Resolver::visit_bool(AST::BoolExpr* expr)
{
    return expr->value;
}

std::any Resolver::visit_array(AST::ArrayExpr* expr)
{
    std::vector<std::any> elements;
    for (const auto& element : expr->elements)
    {
        elements.push_back(resolve_expression(element.get()));
    }
    return elements;
}

std::any Resolver::visit_set(AST::SetExpr* expr)
{
    std::vector<std::any> elements;
    for (const auto& element : expr->elements)
    {
        elements.push_back(resolve_expression(element.get()));
    }
    return elements;
}

std::any Resolver::visit_map(AST::MapExpr* expr)
{
    std::map<std::string, std::any> elements;
    for (const auto& element : expr->elements)
    {
        elements[element.first.lexeme] = resolve_expression(element.second.get());
    }
    return elements;
}

std::any Resolver::visit_color(AST::ColorExpr* expr)
{
    ResolvedColor color;
    color.r = expr->r;
    color.g = expr->g;
    color.b = expr->b;
    return color;
}

std::any Resolver::visit_coord(AST::CoordExpr* expr)
{
    ResolvedCoord coord;
    coord.x = resolve_expression(expr->x.get());
    coord.y = resolve_expression(expr->y.get());
    if (expr->z)
    {
        coord.z = resolve_expression(expr->z.get());
    }
    return coord;
}

std::any Resolver::visit_macro(AST::MacroExpr* expr)
{
    if (m_macros.find(expr->name.lexeme) == m_macros.end())
    {
        throw std::runtime_error("Error at line " + std::to_string(expr->name.line) + ", column " + std::to_string(expr->name.column) + ": Undefined macro: " + expr->name.lexeme);
    }
    return resolve_expression(m_macros[expr->name.lexeme]);
}

std::any Resolver::visit_binary(AST::BinaryExpr* expr)
{
    std::any left = resolve_expression(expr->left.get());
    std::any right = resolve_expression(expr->right.get());

    if (left.type() == typeid(double) && right.type() == typeid(double))
    {
        double left_val = std::any_cast<double>(left);
        double right_val = std::any_cast<double>(right);

        switch (expr->op.type)
        {
            case TokenType::PLUS: return left_val + right_val;
            case TokenType::MINUS: return left_val - right_val;
            case TokenType::STAR: return left_val * right_val;
            case TokenType::SLASH:
                if (right_val == 0) {
                    throw std::runtime_error("Error at line " + std::to_string(expr->op.line) + ", column " + std::to_string(expr->op.column) + ": Division by zero.");
                }
                return left_val / right_val;
            case TokenType::PERCENT:
                if (right_val == 0) {
                    throw std::runtime_error("Error at line " + std::to_string(expr->op.line) + ", column " + std::to_string(expr->op.column) + ": Division by zero.");
                }
                return fmod(left_val, right_val);
            default: break;
        }
    }

    throw std::runtime_error("Error at line " + std::to_string(expr->op.line) + ", column " + std::to_string(expr->op.column) + ": Operands must be numbers for arithmetic operations.");
}

std::any Resolver::visit_grouping(AST::GroupingExpr* expr)
{
    return resolve_expression(expr->expression.get());
}

std::any Resolver::visit_cross_section_ref(AST::CrossSectionRefExpr* expr)
{
    std::string section = expr->section.lexeme;
    std::string key = expr->key.lexeme;
    if (m_resolved_config.find(section) == m_resolved_config.end() ||
        m_resolved_config.at(section).find(key) == m_resolved_config.at(section).end())
    {
        throw std::runtime_error("Error at line " + std::to_string(expr->section.line) + ", column " + std::to_string(expr->section.column) + ": Undefined cross-section reference: " + section + "." + key);
    }
    return m_resolved_config.at(section).at(key);
}

std::any Resolver::visit_env_var_ref(AST::EnvVarRefExpr* expr)
{
    const char* value = std::getenv(expr->name.lexeme.c_str());
    if (value == nullptr)
    {
        return std::string(""); // Return empty string if not found
    }
    return std::string(value);
}

void Resolver::visit_quick_reg(AST::QuickRegStmt* stmt)
{
    if (m_current_section.empty())
    {
        throw std::runtime_error("Quick registration '+=' can only be used inside a section.");
    }

    int index = m_quick_reg_indices[m_current_section]++;
    std::string key = std::to_string(index);
    m_resolved_config[m_current_section][key] = resolve_expression(stmt->value.get());
}

std::any Resolver::visit_path(AST::PathExpr* expr)
{
    return expr->path;
}

std::any Resolver::visit_list(AST::ListExpr* expr)
{
    std::vector<std::any> elements;
    for (const auto& element : expr->elements)
    {
        elements.push_back(resolve_expression(element.get()));
    }
    return elements;
}

} // namespace YINI