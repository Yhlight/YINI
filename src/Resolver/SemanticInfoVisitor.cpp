#include "SemanticInfoVisitor.h"
#include "Resolver/Resolver.h"
#include "Ymeta/YmetaManager.h"

namespace YINI
{

SemanticInfoVisitor::SemanticInfoVisitor(const std::string &source, const std::string &uri)
    : m_source(source), m_uri(uri)
{
    m_result["tokens"] = nlohmann::json::array();
    m_result["symbols"] = nlohmann::json::array();
    m_result["diagnostics"] = nlohmann::json::array();
}

nlohmann::json SemanticInfoVisitor::get_info()
{
    return m_result;
}

void SemanticInfoVisitor::add_token(const Token &token, const std::string &type, const std::string &modifiers,
                                  const std::string &hover_text)
{
    nlohmann::json token_info;
    token_info["line"] = token.line - 1;
    token_info["startChar"] = token.column - 1;
    token_info["length"] = token.lexeme.length();
    token_info["tokenType"] = type;
    token_info["tokenModifiers"] = modifiers;
    if (!hover_text.empty())
    {
        token_info["hoverText"] = hover_text;
    }
    m_result["tokens"].push_back(token_info);
}

namespace
{
// Helper to get a string representation of the type held in a YiniVariant
std::string get_variant_type_name(const YiniVariant &v)
{
    return std::visit(
        [](auto &&arg) -> std::string
        {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::monostate>)
                return "null";
            else if constexpr (std::is_same_v<T, int64_t>)
                return "int";
            else if constexpr (std::is_same_v<T, double>)
                return "float";
            else if constexpr (std::is_same_v<T, bool>)
                return "bool";
            else if constexpr (std::is_same_v<T, std::string>)
                return "string";
            else if constexpr (std::is_same_v<T, ResolvedColor>)
                return "color";
            else if constexpr (std::is_same_v<T, ResolvedCoord>)
                return "coord";
            else if constexpr (std::is_same_v<T, YiniMap>)
                return "map";
            else if constexpr (std::is_same_v<T, YiniStruct>)
                return "struct";
            else if constexpr (std::is_same_v<T, std::unique_ptr<YiniArray>>)
                return "array";
            else
                return "unknown";
        },
        v);
}
} // namespace

// Visitor methods for expressions
YiniVariant SemanticInfoVisitor::visitLiteralExpr(AST::LiteralExpr *expr)
{
    if (expr->value.type == TokenType::STRING)
    {
        add_token(expr->value, "string");
    }
    else if (expr->value.type == TokenType::NUMBER)
    {
        add_token(expr->value, "number");
    }
    return {};
}

YiniVariant SemanticInfoVisitor::visitBoolExpr(AST::BoolExpr *expr)
{
    // This is a synthetic node, the token is handled by the parser
    return {};
}

YiniVariant SemanticInfoVisitor::visitArrayExpr(AST::ArrayExpr *expr)
{
    for (auto &element : expr->elements)
    {
        element->accept(this);
    }
    return {};
}

YiniVariant SemanticInfoVisitor::visitSetExpr(AST::SetExpr *expr)
{
    for (auto &element : expr->elements)
    {
        element->accept(this);
    }
    return {};
}

YiniVariant SemanticInfoVisitor::visitMapExpr(AST::MapExpr *expr)
{
    for (auto &element : expr->elements)
    {
        add_token(element.first, "property");
        element.second->accept(this);
    }
    return {};
}

YiniVariant SemanticInfoVisitor::visitStructExpr(AST::StructExpr *expr)
{
    add_token(expr->key, "property");
    expr->value->accept(this);
    return {};
}

YiniVariant SemanticInfoVisitor::visitColorExpr(AST::ColorExpr *expr)
{
    return {};
}

YiniVariant SemanticInfoVisitor::visitCoordExpr(AST::CoordExpr *expr)
{
    expr->x->accept(this);
    expr->y->accept(this);
    if (expr->z)
    {
        expr->z->accept(this);
    }
    return {};
}

YiniVariant SemanticInfoVisitor::visitBinaryExpr(AST::BinaryExpr *expr)
{
    expr->left->accept(this);
    add_token(expr->op, "operator");
    expr->right->accept(this);
    return {};
}

YiniVariant SemanticInfoVisitor::visitUnaryExpr(AST::UnaryExpr *expr)
{
    add_token(expr->op, "operator");
    expr->right->accept(this);
    return {};
}

YiniVariant SemanticInfoVisitor::visitGroupingExpr(AST::GroupingExpr *expr)
{
    expr->expression->accept(this);
    return {};
}

YiniVariant SemanticInfoVisitor::visitMacroExpr(AST::MacroExpr *expr)
{
    add_token(expr->name, "macro");
    return {};
}

YiniVariant SemanticInfoVisitor::visitCrossSectionRefExpr(AST::CrossSectionRefExpr *expr)
{
    add_token(expr->section, "namespace");
    add_token(expr->key, "property");
    return {};
}

YiniVariant SemanticInfoVisitor::visitEnvVarRefExpr(AST::EnvVarRefExpr *expr)
{
    add_token(expr->name, "variable", "readonly");
    return {};
}

YiniVariant SemanticInfoVisitor::visitDynaExpr(AST::DynaExpr *expr)
{
    expr->expression->accept(this);
    return {};
}

YiniVariant SemanticInfoVisitor::visitPathExpr(AST::PathExpr *expr)
{
    return {};
}

YiniVariant SemanticInfoVisitor::visitListExpr(AST::ListExpr *expr)
{
    for (auto &element : expr->elements)
    {
        element->accept(this);
    }
    return {};
}

// Visitor methods for statements
void SemanticInfoVisitor::visitKeyValueStmt(AST::KeyValueStmt *stmt)
{
    // To get the resolved type for the hover, we need to resolve this specific value expression.
    // This is a bit inefficient as it creates a mini-resolver, but it's self-contained.
    // A more advanced implementation might pass the resolved config into the visitor.
    YiniVariant resolved_value;
    try
    {
        std::vector<std::unique_ptr<AST::Stmt>> temp_ast; // Empty ast for resolver
        YmetaManager temp_ymeta;
        Resolver temp_resolver(temp_ast, temp_ymeta);
        resolved_value = stmt->value->accept(&temp_resolver);
    }
    catch (...)
    {
        // Ignore resolution errors for semantic token highlighting
    }

    add_token(stmt->key, "property", "", get_variant_type_name(resolved_value));

    nlohmann::json symbol;
    symbol["name"] = stmt->key.lexeme;
    symbol["kind"] = 12; // SymbolKind.Field
    symbol["location"] = {
        {"uri", m_uri},
        {"range",
         {{"start", {{"line", stmt->key.line - 1}, {"character", stmt->key.column - 1}}},
          {"end", {{"line", stmt->key.line - 1}, {"character", stmt->key.column - 1 + stmt->key.lexeme.length()}}}}}};
    m_result["symbols"].push_back(symbol);

    stmt->value->accept(this);
}

void SemanticInfoVisitor::visitSectionStmt(AST::SectionStmt *stmt)
{
    add_token(stmt->name, "class");
    m_current_section = stmt->name.lexeme;

    nlohmann::json symbol;
    symbol["name"] = stmt->name.lexeme;
    symbol["kind"] = 2; // SymbolKind.Namespace
    symbol["location"] = {
        {"uri", m_uri},
        {"range",
         {{"start", {{"line", stmt->name.line - 1}, {"character", stmt->name.column - 1}}},
          {"end",
           {{"line", stmt->name.line - 1}, {"character", stmt->name.column - 1 + stmt->name.lexeme.length()}}}}}};
    m_result["symbols"].push_back(symbol);

    for (auto &parent : stmt->parent_sections)
    {
        add_token(parent, "class", "readonly");
    }

    for (auto &statement : stmt->statements)
    {
        statement->accept(this);
    }
    m_current_section = "";
}

void SemanticInfoVisitor::visitDefineSectionStmt(AST::DefineSectionStmt *stmt)
{
    for (auto &definition : stmt->definitions)
    {
        definition->accept(this);
    }
}

void SemanticInfoVisitor::visitIncludeStmt(AST::IncludeStmt *stmt, bool collection_mode)
{
    for (auto &path : stmt->paths)
    {
        path->accept(this);
    }
}

void SemanticInfoVisitor::visitQuickRegStmt(AST::QuickRegStmt *stmt)
{
    stmt->value->accept(this);
}

void SemanticInfoVisitor::visitSchemaRuleStmt(AST::SchemaRuleStmt *stmt)
{
    add_token(stmt->key, "property", "readonly");
}

void SemanticInfoVisitor::visitSchemaSectionStmt(AST::SchemaSectionStmt *stmt)
{
    add_token(stmt->name, "class", "readonly");
    for (auto &rule : stmt->rules)
    {
        rule->accept(this);
    }
}

void SemanticInfoVisitor::visitSchemaStmt(AST::SchemaStmt *stmt)
{
    for (auto &section : stmt->sections)
    {
        section->accept(this);
    }
}

} // namespace YINI
