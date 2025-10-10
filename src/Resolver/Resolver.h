#pragma once

#include "Parser/AST.h"
#include <vector>
#include <memory>
#include <string>
#include <map>
#include <any>

namespace YINI
{

struct ResolvedColor
{
    uint8_t r, g, b;
};

struct ResolvedCoord
{
    std::any x, y, z;
};

class Resolver
{
public:
    Resolver(const std::vector<std::unique_ptr<AST::Stmt>>& statements);
    std::map<std::string, std::any> resolve();

private:
    void resolve_statement(AST::Stmt* stmt);
    std::any resolve_expression(AST::Expr* expr);

    void visit_define_section(AST::DefineSectionStmt* stmt);
    void visit_section(AST::SectionStmt* stmt);
    void visit_include(AST::IncludeStmt* stmt);
    void visit_key_value(AST::KeyValueStmt* stmt);

    std::any visit_literal(AST::LiteralExpr* expr);
    std::any visit_bool(AST::BoolExpr* expr);
    std::any visit_array(AST::ArrayExpr* expr);
    std::any visit_set(AST::SetExpr* expr);
    std::any visit_map(AST::MapExpr* expr);
    std::any visit_color(AST::ColorExpr* expr);
    std::any visit_coord(AST::CoordExpr* expr);
    std::any visit_macro(AST::MacroExpr* expr);
    std::any visit_binary(AST::BinaryExpr* expr);
    std::any visit_grouping(AST::GroupingExpr* expr);
    std::any visit_cross_section_ref(AST::CrossSectionRefExpr* expr);
    std::any visit_env_var_ref(AST::EnvVarRefExpr* expr);

    const std::vector<std::unique_ptr<AST::Stmt>>& m_statements;
    std::map<std::string, AST::Expr*> m_macros;
    std::map<std::string, std::any> m_resolved_config;
    std::string m_current_section;
};

} // namespace YINI