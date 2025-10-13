#pragma once
#include "Lexer/Token.h"
#include "Parser/AST.h"
#include "Parser/ASTVisitor.h"
#include "nlohmann/json.hpp"
#include <string>
#include <vector>

namespace YINI
{

// A visitor to collect semantic information for the language server
class SemanticInfoVisitor : public AST::ASTVisitor
{
  public:
    SemanticInfoVisitor(const std::string &source, const std::string &uri);
    nlohmann::json get_info();

    // Visitor methods for expressions
    YiniVariant visitLiteralExpr(AST::LiteralExpr *expr) override;
    YiniVariant visitBoolExpr(AST::BoolExpr *expr) override;
    YiniVariant visitArrayExpr(AST::ArrayExpr *expr) override;
    YiniVariant visitSetExpr(AST::SetExpr *expr) override;
    YiniVariant visitMapExpr(AST::MapExpr *expr) override;
    YiniVariant visitStructExpr(AST::StructExpr *expr) override;
    YiniVariant visitColorExpr(AST::ColorExpr *expr) override;
    YiniVariant visitCoordExpr(AST::CoordExpr *expr) override;
    YiniVariant visitBinaryExpr(AST::BinaryExpr *expr) override;
    YiniVariant visitUnaryExpr(AST::UnaryExpr *expr) override;
    YiniVariant visitGroupingExpr(AST::GroupingExpr *expr) override;
    YiniVariant visitMacroExpr(AST::MacroExpr *expr) override;
    YiniVariant visitCrossSectionRefExpr(AST::CrossSectionRefExpr *expr) override;
    YiniVariant visitEnvVarRefExpr(AST::EnvVarRefExpr *expr) override;
    YiniVariant visitDynaExpr(AST::DynaExpr *expr) override;
    YiniVariant visitPathExpr(AST::PathExpr *expr) override;
    YiniVariant visitListExpr(AST::ListExpr *expr) override;

    // Visitor methods for statements
    void visitKeyValueStmt(AST::KeyValueStmt *stmt) override;
    void visitSectionStmt(AST::SectionStmt *stmt) override;
    void visitDefineSectionStmt(AST::DefineSectionStmt *stmt) override;
    void visitIncludeStmt(AST::IncludeStmt *stmt, bool collection_mode) override;
    void visitQuickRegStmt(AST::QuickRegStmt *stmt) override;
    void visitSchemaRuleStmt(AST::SchemaRuleStmt *stmt) override;
    void visitSchemaSectionStmt(AST::SchemaSectionStmt *stmt) override;
    void visitSchemaStmt(AST::SchemaStmt *stmt) override;

  private:
    void add_token(const Token &token, const std::string &type, const std::string &modifiers = "",
                   const std::string &hover_text = "");

    const std::string &m_source;
    std::string m_uri;
    nlohmann::json m_result;
    std::string m_current_section;
};

} // namespace YINI
