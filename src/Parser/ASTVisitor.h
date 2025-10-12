#pragma once
#include "YiniTypes.h"

namespace YINI
{
namespace AST
{

// Forward declarations for all AST node types
struct Node;
struct Expr;
struct Stmt;
struct LiteralExpr;
struct BoolExpr;
struct ArrayExpr;
struct SetExpr;
struct MapExpr;
struct ColorExpr;
struct CoordExpr;
struct BinaryExpr;
struct UnaryExpr;
struct GroupingExpr;
struct KeyValueStmt;
struct SectionStmt;
struct DefineSectionStmt;
struct IncludeStmt;
struct MacroExpr;
struct CrossSectionRefExpr;
struct EnvVarRefExpr;
struct DynaExpr;
struct PathExpr;
struct ListExpr;
struct QuickRegStmt;
struct SchemaRuleStmt;
struct SchemaSectionStmt;
struct SchemaStmt;

class ASTVisitor
{
public:
    virtual ~ASTVisitor() = default;

    // Visitor methods for expressions
    virtual YiniVariant visitLiteralExpr(LiteralExpr* expr) = 0;
    virtual YiniVariant visitBoolExpr(BoolExpr* expr) = 0;
    virtual YiniVariant visitArrayExpr(ArrayExpr* expr) = 0;
    virtual YiniVariant visitSetExpr(SetExpr* expr) = 0;
    virtual YiniVariant visitMapExpr(MapExpr* expr) = 0;
    virtual YiniVariant visitColorExpr(ColorExpr* expr) = 0;
    virtual YiniVariant visitCoordExpr(CoordExpr* expr) = 0;
    virtual YiniVariant visitBinaryExpr(BinaryExpr* expr) = 0;
    virtual YiniVariant visitUnaryExpr(UnaryExpr* expr) = 0;
    virtual YiniVariant visitGroupingExpr(GroupingExpr* expr) = 0;
    virtual YiniVariant visitMacroExpr(MacroExpr* expr) = 0;
    virtual YiniVariant visitCrossSectionRefExpr(CrossSectionRefExpr* expr) = 0;
    virtual YiniVariant visitEnvVarRefExpr(EnvVarRefExpr* expr) = 0;
    virtual YiniVariant visitDynaExpr(DynaExpr* expr) = 0;
    virtual YiniVariant visitPathExpr(PathExpr* expr) = 0;
    virtual YiniVariant visitListExpr(ListExpr* expr) = 0;

    // Visitor methods for statements
    virtual void visitKeyValueStmt(KeyValueStmt* stmt) = 0;
    virtual void visitSectionStmt(SectionStmt* stmt) = 0;
    virtual void visitDefineSectionStmt(DefineSectionStmt* stmt) = 0;
    virtual void visitIncludeStmt(IncludeStmt* stmt, bool collection_mode) = 0;
    virtual void visitQuickRegStmt(QuickRegStmt* stmt) = 0;
    virtual void visitSchemaRuleStmt(SchemaRuleStmt* stmt) = 0;
    virtual void visitSchemaSectionStmt(SchemaSectionStmt* stmt) = 0;
    virtual void visitSchemaStmt(SchemaStmt* stmt) = 0;
};

} // namespace AST
} // namespace YINI
