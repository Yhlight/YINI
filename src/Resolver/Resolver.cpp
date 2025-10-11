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

std::map<std::string, std::any> Resolver::resolve()
{
    // First pass: collect all section definitions and macros from the root and included files.
    collect_declarations(m_statements);

    // Second pass: resolve each section, handling inheritance.
    for (const auto& pair : m_section_nodes)
    {
        resolve_section(pair.first);
    }

    // Third pass: flatten the resolved per-section data into the final config map.
    for (const auto& section_pair : m_resolved_sections_data)
    {
        const std::string& section_name = section_pair.first;
        const auto& section_data = section_pair.second;
        for (const auto& value_pair : section_data)
        {
            const std::string& key = value_pair.first;
            const std::any& value = value_pair.second;
            m_resolved_config[section_name + "." + key] = value;
        }
    }

    return m_resolved_config;
}

void Resolver::collect_declarations(const std::vector<std::unique_ptr<AST::Stmt>>& statements)
{
    for (const auto& stmt : statements)
    {
        if (auto* define_stmt = dynamic_cast<AST::DefineSectionStmt*>(stmt.get()))
        {
            for (const auto& definition : define_stmt->definitions)
            {
                m_macros[definition->key.lexeme] = definition->value.get();
            }
        }
        else if (auto* section_stmt = dynamic_cast<AST::SectionStmt*>(stmt.get()))
        {
            const std::string& name = section_stmt->name.lexeme;
            if (m_section_nodes.find(name) == m_section_nodes.end())
            {
                m_section_nodes[name] = section_stmt;
            }
            else
            {
                // Merge statements if the section is defined multiple times.
                m_section_nodes[name]->statements.insert(
                    m_section_nodes[name]->statements.end(),
                    std::make_move_iterator(section_stmt->statements.begin()),
                    std::make_move_iterator(section_stmt->statements.end())
                );
            }
        }
        else if (auto* include_stmt = dynamic_cast<AST::IncludeStmt*>(stmt.get()))
        {
            visitIncludeStmt(include_stmt, true); // Pass true to indicate collection mode.
        }
    }
}

std::map<std::string, std::any> Resolver::resolve_section(const std::string& section_name)
{
    // If already resolved, return the cached data.
    if (m_resolved_sections_data.count(section_name))
    {
        return m_resolved_sections_data[section_name];
    }

    // Check for circular dependencies.
    if (m_resolving_stack.count(section_name))
    {
        throw std::runtime_error("Circular inheritance detected involving section: " + section_name);
    }

    if (m_section_nodes.find(section_name) == m_section_nodes.end())
    {
        throw std::runtime_error("Reference to undefined section: " + section_name);
    }

    m_resolving_stack.insert(section_name);

    m_current_section_name = section_name; // Set context for DynaExpr
    AST::SectionStmt* section_stmt = m_section_nodes[section_name];
    std::map<std::string, std::any> section_data;

    // Resolve and merge parent sections first.
    for (const auto& parent_token : section_stmt->parent_sections)
    {
        const std::string& parent_name = parent_token.lexeme;
        auto parent_data = resolve_section(parent_name);
        for (const auto& pair : parent_data)
        {
            section_data[pair.first] = pair.second;
        }
    }

    // Resolve this section's own key-value pairs, overriding any from parents.
    m_current_section_data = &section_data;
    for (const auto& statement : section_stmt->statements)
    {
        statement->accept(this);
    }
    m_current_section_data = nullptr;

    m_resolving_stack.erase(section_name);
    m_resolved_sections_data[section_name] = section_data;
    return section_data;
}

void Resolver::visitDefineSectionStmt(AST::DefineSectionStmt* stmt)
{
    // This is handled in the collection pass.
}

void Resolver::visitSectionStmt(AST::SectionStmt* stmt)
{
    // This is handled by the resolve_section method.
}

void Resolver::visitIncludeStmt(AST::IncludeStmt* stmt, bool collection_mode)
{
    for (const auto& path_expr : stmt->paths)
    {
        std::any path_any = path_expr->accept(this);
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

        if (collection_mode)
        {
            collect_declarations(included_ast);
            m_included_asts.push_back(std::move(included_ast));
        }
        // else: includes are fully processed during the collection pass,
        // so there's nothing to do during the resolution pass.
    }
}

void Resolver::visitKeyValueStmt(AST::KeyValueStmt* stmt)
{
    if (!m_current_section_data) return; // Should not happen during the resolution pass.

    std::string key = stmt->key.lexeme;
    std::string full_key = m_current_section_name + "." + key;

    if (auto* dyna_expr = dynamic_cast<AST::DynaExpr*>(stmt->value.get()))
    {
        if (m_ymeta_manager.has_value(full_key))
        {
            (*m_current_section_data)[key] = m_ymeta_manager.get_value(full_key);
        }
        else
        {
            std::any value = dyna_expr->expression->accept(this);
            m_ymeta_manager.set_value(full_key, value);
            (*m_current_section_data)[key] = value;
        }
    }
    else
    {
        (*m_current_section_data)[key] = stmt->value->accept(this);
    }
}

void Resolver::visitQuickRegStmt(AST::QuickRegStmt* stmt)
{
    if (!m_current_section_data)
    {
        throw std::runtime_error("Quick registration '+=' can only be used inside a section.");
    }

    int max_index = -1;
    for (const auto& pair : *m_current_section_data) {
        try {
            int current_key = std::stoi(pair.first);
            if (current_key > max_index) {
                max_index = current_key;
            }
        } catch (const std::invalid_argument&) {
            // Ignore keys that are not integers
        }
    }

    std::string key = std::to_string(max_index + 1);
    (*m_current_section_data)[key] = stmt->value->accept(this);
}

void Resolver::visitSchemaRuleStmt(AST::SchemaRuleStmt* stmt)
{
    // The resolver does not handle schema validation.
}

void Resolver::visitSchemaSectionStmt(AST::SchemaSectionStmt* stmt)
{
    // The resolver does not handle schema validation.
}

void Resolver::visitSchemaStmt(AST::SchemaStmt* stmt)
{
    // The resolver does not handle schema validation.
}

std::any Resolver::visitLiteralExpr(AST::LiteralExpr* expr)
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

std::any Resolver::visitBoolExpr(AST::BoolExpr* expr)
{
    return expr->value;
}

std::any Resolver::visitArrayExpr(AST::ArrayExpr* expr)
{
    std::vector<std::any> elements;
    for (const auto& element : expr->elements)
    {
        elements.push_back(element->accept(this));
    }
    return elements;
}

std::any Resolver::visitSetExpr(AST::SetExpr* expr)
{
    std::vector<std::any> elements;
    for (const auto& element : expr->elements)
    {
        elements.push_back(element->accept(this));
    }
    return elements;
}

std::any Resolver::visitMapExpr(AST::MapExpr* expr)
{
    std::map<std::string, std::any> elements;
    for (const auto& element : expr->elements)
    {
        elements[element.first.lexeme] = element.second->accept(this);
    }
    return elements;
}

std::any Resolver::visitColorExpr(AST::ColorExpr* expr)
{
    ResolvedColor color;
    color.r = expr->r;
    color.g = expr->g;
    color.b = expr->b;
    return color;
}

std::any Resolver::visitCoordExpr(AST::CoordExpr* expr)
{
    ResolvedCoord coord;
    coord.x = expr->x->accept(this);
    coord.y = expr->y->accept(this);
    if (expr->z)
    {
        coord.z = expr->z->accept(this);
    }
    return coord;
}

std::any Resolver::visitMacroExpr(AST::MacroExpr* expr)
{
    if (m_macros.find(expr->name.lexeme) == m_macros.end())
    {
        throw std::runtime_error("Error at line " + std::to_string(expr->name.line) + ", column " + std::to_string(expr->name.column) + ": Undefined macro: " + expr->name.lexeme);
    }
    return m_macros[expr->name.lexeme]->accept(this);
}

std::any Resolver::visitBinaryExpr(AST::BinaryExpr* expr)
{
    std::any left = expr->left->accept(this);
    std::any right = expr->right->accept(this);

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

std::any Resolver::visitUnaryExpr(AST::UnaryExpr* expr)
{
    std::any right = expr->right->accept(this);

    if (expr->op.type == TokenType::MINUS)
    {
        if (right.type() == typeid(double))
        {
            return -std::any_cast<double>(right);
        }
        else
        {
            throw std::runtime_error("Error at line " + std::to_string(expr->op.line) + ", column " + std::to_string(expr->op.column) + ": Operand must be a number for unary minus.");
        }
    }

    // Should not be reached for other unary operators if any are added later.
    return {};
}

std::any Resolver::visitGroupingExpr(AST::GroupingExpr* expr)
{
    return expr->expression->accept(this);
}

std::any Resolver::visitCrossSectionRefExpr(AST::CrossSectionRefExpr* expr)
{
    const std::string& section_name = expr->section.lexeme;
    const std::string& key = expr->key.lexeme;

    // Resolve the referenced section if it hasn't been already.
    resolve_section(section_name);

    const auto& section_data = m_resolved_sections_data.at(section_name);
    if (section_data.find(key) == section_data.end())
    {
        throw std::runtime_error("Error at line " + std::to_string(expr->key.line) + ", column " + std::to_string(expr->key.column) + ": Undefined key '" + key + "' in section '" + section_name + "'.");
    }
    return section_data.at(key);
}

std::any Resolver::visitEnvVarRefExpr(AST::EnvVarRefExpr* expr)
{
    const char* value = std::getenv(expr->name.lexeme.c_str());
    if (value == nullptr)
    {
        return std::string(""); // Return empty string if not found
    }
    return std::string(value);
}

std::any Resolver::visitDynaExpr(AST::DynaExpr* expr)
{
    // DynaExpr needs to know the fully resolved key to interact with YmetaManager.
    // This logic is now implicitly handled by visitKeyValueStmt, as DynaExpr
    // is just a wrapper. The value resolution happens there.
    return expr->expression->accept(this);
}

std::any Resolver::visitPathExpr(AST::PathExpr* expr)
{
    return expr->path;
}

std::any Resolver::visitListExpr(AST::ListExpr* expr)
{
    std::vector<std::any> elements;
    for (const auto& element : expr->elements)
    {
        elements.push_back(element->accept(this));
    }
    return elements;
}

} // namespace YINI