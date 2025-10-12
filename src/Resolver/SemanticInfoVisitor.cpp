#include "SemanticInfoVisitor.h"

namespace YINI
{

SemanticInfoVisitor::SemanticInfoVisitor(const std::string& source, const std::string& uri) : m_source(source), m_uri(uri)
{
    m_result["tokens"] = nlohmann::json::array();
    m_result["symbols"] = nlohmann::json::array();
    m_result["diagnostics"] = nlohmann::json::array();
}

nlohmann::json SemanticInfoVisitor::get_info()
{
    return m_result;
}

void SemanticInfoVisitor::add_token(const Token& token, const std::string& type, const std::string& modifiers)
{
    nlohmann::json token_info;
    token_info["line"] = token.line -1;
    token_info["startChar"] = token.column -1;
    token_info["length"] = token.lexeme.length();
    token_info["tokenType"] = type;
    token_info["tokenModifiers"] = modifiers;
    m_result["tokens"].push_back(token_info);
}

// Visitor methods for expressions
std::any SemanticInfoVisitor::visitLiteralExpr(AST::LiteralExpr* expr)
{
    if (expr->value.type == TokenType::STRING) {
        add_token(expr->value, "string");
    } else if (expr->value.type == TokenType::NUMBER) {
        add_token(expr->value, "number");
    }
    return {};
}

std::any SemanticInfoVisitor::visitBoolExpr(AST::BoolExpr* expr)
{
    // This is a synthetic node, the token is handled by the parser
    return {};
}

std::any SemanticInfoVisitor::visitArrayExpr(AST::ArrayExpr* expr)
{
    for (auto& element : expr->elements) {
        element->accept(this);
    }
    return {};
}

std::any SemanticInfoVisitor::visitSetExpr(AST::SetExpr* expr)
{
    for (auto& element : expr->elements) {
        element->accept(this);
    }
    return {};
}

std::any SemanticInfoVisitor::visitMapExpr(AST::MapExpr* expr)
{
    for (auto& element : expr->elements) {
        add_token(element.first, "property");
        element.second->accept(this);
    }
    return {};
}

std::any SemanticInfoVisitor::visitColorExpr(AST::ColorExpr* expr)
{
    return {};
}

std::any SemanticInfoVisitor::visitCoordExpr(AST::CoordExpr* expr)
{
    expr->x->accept(this);
    expr->y->accept(this);
    if (expr->z) {
        expr->z->accept(this);
    }
    return {};
}

std::any SemanticInfoVisitor::visitBinaryExpr(AST::BinaryExpr* expr)
{
    expr->left->accept(this);
    add_token(expr->op, "operator");
    expr->right->accept(this);
    return {};
}

std::any SemanticInfoVisitor::visitUnaryExpr(AST::UnaryExpr* expr)
{
    add_token(expr->op, "operator");
    expr->right->accept(this);
    return {};
}

std::any SemanticInfoVisitor::visitGroupingExpr(AST::GroupingExpr* expr)
{
    expr->expression->accept(this);
    return {};
}

std::any SemanticInfoVisitor::visitMacroExpr(AST::MacroExpr* expr)
{
    add_token(expr->name, "macro");
    return {};
}

std::any SemanticInfoVisitor::visitCrossSectionRefExpr(AST::CrossSectionRefExpr* expr)
{
    add_token(expr->section, "namespace");
    add_token(expr->key, "property");
    return {};
}

std::any SemanticInfoVisitor::visitEnvVarRefExpr(AST::EnvVarRefExpr* expr)
{
    add_token(expr->name, "variable", "readonly");
    return {};
}

std::any SemanticInfoVisitor::visitDynaExpr(AST::DynaExpr* expr)
{
    expr->expression->accept(this);
    return {};
}

std::any SemanticInfoVisitor::visitPathExpr(AST::PathExpr* expr)
{
    return {};
}

std::any SemanticInfoVisitor::visitListExpr(AST::ListExpr* expr)
{
    for (auto& element : expr->elements) {
        element->accept(this);
    }
    return {};
}

// Visitor methods for statements
void SemanticInfoVisitor::visitKeyValueStmt(AST::KeyValueStmt* stmt)
{
    add_token(stmt->key, "property");

    nlohmann::json symbol;
    symbol["name"] = stmt->key.lexeme;
    symbol["kind"] = 12; // SymbolKind.Field
    symbol["location"] = {
        {"uri", m_uri},
        {"range", {
            {"start", {{"line", stmt->key.line - 1}, {"character", stmt->key.column - 1}}},
            {"end", {{"line", stmt->key.line - 1}, {"character", stmt->key.column - 1 + stmt->key.lexeme.length()}}}
        }}
    };
    m_result["symbols"].push_back(symbol);

    stmt->value->accept(this);
}

void SemanticInfoVisitor::visitSectionStmt(AST::SectionStmt* stmt)
{
    add_token(stmt->name, "class");
    m_current_section = stmt->name.lexeme;

    nlohmann::json symbol;
    symbol["name"] = stmt->name.lexeme;
    symbol["kind"] = 2; // SymbolKind.Namespace
    symbol["location"] = {
        {"uri", m_uri},
        {"range", {
            {"start", {{"line", stmt->name.line - 1}, {"character", stmt->name.column - 1}}},
            {"end", {{"line", stmt->name.line - 1}, {"character", stmt->name.column - 1 + stmt->name.lexeme.length()}}}
        }}
    };
    m_result["symbols"].push_back(symbol);

    for(auto& parent : stmt->parent_sections) {
        add_token(parent, "class", "readonly");
    }

    for (auto& statement : stmt->statements) {
        statement->accept(this);
    }
    m_current_section = "";
}

void SemanticInfoVisitor::visitDefineSectionStmt(AST::DefineSectionStmt* stmt)
{
    for (auto& definition : stmt->definitions) {
        definition->accept(this);
    }
}

void SemanticInfoVisitor::visitIncludeStmt(AST::IncludeStmt* stmt, bool collection_mode)
{
    for (auto& path : stmt->paths) {
        path->accept(this);
    }
}

void SemanticInfoVisitor::visitQuickRegStmt(AST::QuickRegStmt* stmt)
{
    stmt->value->accept(this);
}

void SemanticInfoVisitor::visitSchemaRuleStmt(AST::SchemaRuleStmt* stmt)
{
    add_token(stmt->key, "property", "readonly");
}

void SemanticInfoVisitor::visitSchemaSectionStmt(AST::SchemaSectionStmt* stmt)
{
    add_token(stmt->name, "class", "readonly");
    for (auto& rule : stmt->rules) {
        rule->accept(this);
    }
}

void SemanticInfoVisitor::visitSchemaStmt(AST::SchemaStmt* stmt)
{
    for (auto& section : stmt->sections) {
        section->accept(this);
    }
}

} // namespace YINI
