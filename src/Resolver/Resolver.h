#pragma once

#include "Parser/AST.h"
#include "Parser/ASTVisitor.h"
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

class Resolver : public AST::ASTVisitor
{
public:
    Resolver(const std::vector<std::unique_ptr<AST::Stmt>>& statements, YmetaManager& ymeta_manager);
    std::map<std::string, std::any> resolve();

private:
    // Visitor methods for expressions
    std::any visitLiteralExpr(AST::LiteralExpr* expr) override;
    std::any visitBoolExpr(AST::BoolExpr* expr) override;
    std::any visitArrayExpr(AST::ArrayExpr* expr) override;
    std::any visitSetExpr(AST::SetExpr* expr) override;
    std::any visitMapExpr(AST::MapExpr* expr) override;
    std::any visitColorExpr(AST::ColorExpr* expr) override;
    std::any visitCoordExpr(AST::CoordExpr* expr) override;
    std::any visitBinaryExpr(AST::BinaryExpr* expr) override;
    std::any visitUnaryExpr(AST::UnaryExpr* expr) override;
    std::any visitGroupingExpr(AST::GroupingExpr* expr) override;
    std::any visitMacroExpr(AST::MacroExpr* expr) override;
    std::any visitCrossSectionRefExpr(AST::CrossSectionRefExpr* expr) override;
    std::any visitEnvVarRefExpr(AST::EnvVarRefExpr* expr) override;
    std::any visitDynaExpr(AST::DynaExpr* expr) override;
    std::any visitPathExpr(AST::PathExpr* expr) override;
    std::any visitListExpr(AST::ListExpr* expr) override;

    // Visitor methods for statements
    void visitKeyValueStmt(AST::KeyValueStmt* stmt) override;
    void visitSectionStmt(AST::SectionStmt* stmt) override;
    void visitDefineSectionStmt(AST::DefineSectionStmt* stmt) override;
    void visitIncludeStmt(AST::IncludeStmt* stmt, bool collection_mode = false);
    void visitQuickRegStmt(AST::QuickRegStmt* stmt) override;
    void visitSchemaRuleStmt(AST::SchemaRuleStmt* stmt) override;
    void visitSchemaSectionStmt(AST::SchemaSectionStmt* stmt) override;
    void visitSchemaStmt(AST::SchemaStmt* stmt) override;

    void collect_declarations(const std::vector<std::unique_ptr<AST::Stmt>>& statements);
    std::map<std::string, std::any> resolve_section(const std::string& section_name);

    const std::vector<std::unique_ptr<AST::Stmt>>& m_statements;
    YmetaManager& m_ymeta_manager;
    std::map<std::string, AST::Expr*> m_macros;
    std::map<std::string, std::any> m_resolved_config;
    std::map<std::string, int> m_quick_reg_indices;

    // Data structures for the new multi-pass approach
    std::string m_current_section_name;
    std::map<std::string, AST::SectionStmt*> m_section_nodes;
    std::map<std::string, std::map<std::string, std::any>> m_resolved_sections_data;
    std::map<std::string, std::any>* m_current_section_data = nullptr;
    std::set<std::string> m_resolving_stack;
    std::vector<std::vector<std::unique_ptr<AST::Stmt>>> m_included_asts;
};

} // namespace YINI