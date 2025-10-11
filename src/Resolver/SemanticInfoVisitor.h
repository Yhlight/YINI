#pragma once
#include "Parser/ASTVisitor.h"
#include "Parser/AST.h"
#include "Lexer/Token.h"
#include "nlohmann/json.hpp"
#include <vector>
#include <string>

namespace YINI
{

// A visitor to collect semantic information for the language server
class SemanticInfoVisitor : public AST::ASTVisitor
{
public:
    SemanticInfoVisitor(const std::string& source);
    nlohmann::json get_info();

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
    void visitIncludeStmt(AST::IncludeStmt* stmt, bool collection_mode) override;
    void visitQuickRegStmt(AST::QuickRegStmt* stmt) override;
    void visitSchemaRuleStmt(AST::SchemaRuleStmt* stmt) override;
    void visitSchemaSectionStmt(AST::SchemaSectionStmt* stmt) override;
    void visitSchemaStmt(AST::SchemaStmt* stmt) override;

private:
    void add_token(const Token& token, const std::string& type, const std::string& modifiers = "");

    const std::string& m_source;
    nlohmann::json m_result;
    std::string m_current_section;
};

} // namespace YINI
