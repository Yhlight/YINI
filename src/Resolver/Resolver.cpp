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
    for (const auto& stmt : m_statements)
    {
        stmt->accept(this);
    }
    return m_resolved_config;
}

void Resolver::visitDefineSectionStmt(AST::DefineSectionStmt* stmt)
{
    for (const auto& definition : stmt->definitions)
    {
        m_macros[definition->key.lexeme] = definition->value.get();
    }
}

void Resolver::visitSectionStmt(AST::SectionStmt* stmt)
{
    m_current_section = stmt->name.lexeme;
    for (const auto& statement : stmt->statements)
    {
        statement->accept(this);
    }
    m_current_section.clear();
}

void Resolver::visitIncludeStmt(AST::IncludeStmt* stmt)
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

        for (const auto& included_stmt : included_ast)
        {
            included_stmt->accept(this);
        }
    }
}

void Resolver::visitKeyValueStmt(AST::KeyValueStmt* stmt)
{
    std::string key = m_current_section.empty() ? stmt->key.lexeme : m_current_section + "." + stmt->key.lexeme;

    if (auto* dyna_expr = dynamic_cast<AST::DynaExpr*>(stmt->value.get()))
    {
        if (m_ymeta_manager.has_value(key))
        {
            m_resolved_config[key] = m_ymeta_manager.get_value(key);
        }
        else
        {
            std::any value = dyna_expr->expression->accept(this);
            m_ymeta_manager.set_value(key, value);
            m_resolved_config[key] = value;
        }
    }
    else
    {
        m_resolved_config[key] = stmt->value->accept(this);
    }
}

void Resolver::visitQuickRegStmt(AST::QuickRegStmt* stmt)
{
    if (m_current_section.empty())
    {
        throw std::runtime_error("Quick registration '+=' can only be used inside a section.");
    }

    int index = m_quick_reg_indices[m_current_section]++;
    std::string key = m_current_section + "." + std::to_string(index);
    m_resolved_config[key] = stmt->value->accept(this);
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
    std::string key = expr->section.lexeme + "." + expr->key.lexeme;
    if (m_resolved_config.find(key) == m_resolved_config.end())
    {
        throw std::runtime_error("Error at line " + std::to_string(expr->section.line) + ", column " + std::to_string(expr->section.column) + ": Undefined cross-section reference: " + key);
    }
    return m_resolved_config[key];
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