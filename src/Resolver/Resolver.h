#pragma once

#include "Parser/AST.h"
#include "Parser/ASTVisitor.h"
#include "Ymeta/YmetaManager.h"
#include <vector>
#include <memory>
#include <string>
#include <map>
#include <set>
#include "YiniTypes.h"

namespace YINI
{

class Resolver : public AST::ASTVisitor
{
public:
    Resolver(const std::vector<std::unique_ptr<AST::Stmt>>& statements, YmetaManager& ymeta_manager);
    std::map<std::string, YiniVariant> resolve();

private:
    // Visitor methods for expressions
    YiniVariant visitLiteralExpr(AST::LiteralExpr* expr) override;
    YiniVariant visitBoolExpr(AST::BoolExpr* expr) override;
    YiniVariant visitArrayExpr(AST::ArrayExpr* expr) override;
    YiniVariant visitSetExpr(AST::SetExpr* expr) override;
    YiniVariant visitMapExpr(AST::MapExpr* expr) override;
    YiniVariant visitColorExpr(AST::ColorExpr* expr) override;
    YiniVariant visitCoordExpr(AST::CoordExpr* expr) override;
    YiniVariant visitBinaryExpr(AST::BinaryExpr* expr) override;
    YiniVariant visitUnaryExpr(AST::UnaryExpr* expr) override;
    YiniVariant visitGroupingExpr(AST::GroupingExpr* expr) override;
    YiniVariant visitMacroExpr(AST::MacroExpr* expr) override;
    YiniVariant visitCrossSectionRefExpr(AST::CrossSectionRefExpr* expr) override;
    YiniVariant visitEnvVarRefExpr(AST::EnvVarRefExpr* expr) override;
    YiniVariant visitDynaExpr(AST::DynaExpr* expr) override;
    YiniVariant visitPathExpr(AST::PathExpr* expr) override;
    YiniVariant visitListExpr(AST::ListExpr* expr) override;

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
    std::map<std::string, YiniVariant> resolve_section(const std::string& section_name);

    const std::vector<std::unique_ptr<AST::Stmt>>& m_statements;
    YmetaManager& m_ymeta_manager;
    std::map<std::string, AST::Expr*> m_macros;
    std::map<std::string, YiniVariant> m_resolved_config;
    std::map<std::string, int> m_quick_reg_indices;

    // Data structures for the new multi-pass approach
    std::string m_current_section_name;
    std::map<std::string, AST::SectionStmt*> m_section_nodes;
    std::map<std::string, std::map<std::string, YiniVariant>> m_resolved_sections_data;
    std::map<std::string, YiniVariant>* m_current_section_data = nullptr;
    std::set<std::string> m_resolving_stack;
    std::vector<std::vector<std::unique_ptr<AST::Stmt>>> m_included_asts;
};

} // namespace YINI
