#pragma once

#include "Parser/AST.h"
#include "Ymeta/YmetaManager.h"
#include <vector>
#include <memory>
#include <string>
#include <map>
#include <any>
#include <set>
#include "YiniTypes.h"

namespace YINI
{

class Resolver
{
public:
    using ResolvedConfig = std::map<std::string, std::map<std::string, std::any>>;
    Resolver(const std::vector<std::unique_ptr<AST::Stmt>>& statements, YmetaManager& ymeta_manager);
    ResolvedConfig resolve();

private:
    void resolve_statement(AST::Stmt* stmt);
    std::any resolve_expression(AST::Expr* expr);

    void visit_define_section(AST::DefineSectionStmt* stmt);
    void visit_section(AST::SectionStmt* stmt);
    void visit_include(AST::IncludeStmt* stmt);
    void visit_key_value(AST::KeyValueStmt* stmt);
    void visit_quick_reg(AST::QuickRegStmt* stmt);

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
    std::any visit_path(AST::PathExpr* expr);
    std::any visit_list(AST::ListExpr* expr);

    const std::vector<std::unique_ptr<AST::Stmt>>& m_statements;
    YmetaManager& m_ymeta_manager;
    std::map<std::string, AST::Expr*> m_macros;
    std::map<std::string, AST::SectionStmt*> m_sections;
    ResolvedConfig m_resolved_config;
    std::map<std::string, int> m_quick_reg_indices;
    std::string m_current_section;
    std::set<std::string> m_resolved_sections;
    std::set<std::string> m_resolving_stack;
};

} // namespace YINI