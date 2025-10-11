#pragma once
#include <any>

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
    virtual std::any visitLiteralExpr(LiteralExpr* expr) = 0;
    virtual std::any visitBoolExpr(BoolExpr* expr) = 0;
    virtual std::any visitArrayExpr(ArrayExpr* expr) = 0;
    virtual std::any visitSetExpr(SetExpr* expr) = 0;
    virtual std::any visitMapExpr(MapExpr* expr) = 0;
    virtual std::any visitColorExpr(ColorExpr* expr) = 0;
    virtual std::any visitCoordExpr(CoordExpr* expr) = 0;
    virtual std::any visitBinaryExpr(BinaryExpr* expr) = 0;
    virtual std::any visitUnaryExpr(UnaryExpr* expr) = 0;
    virtual std::any visitGroupingExpr(GroupingExpr* expr) = 0;
    virtual std::any visitMacroExpr(MacroExpr* expr) = 0;
    virtual std::any visitCrossSectionRefExpr(CrossSectionRefExpr* expr) = 0;
    virtual std::any visitEnvVarRefExpr(EnvVarRefExpr* expr) = 0;
    virtual std::any visitDynaExpr(DynaExpr* expr) = 0;
    virtual std::any visitPathExpr(PathExpr* expr) = 0;
    virtual std::any visitListExpr(ListExpr* expr) = 0;

    // Visitor methods for statements
    virtual void visitKeyValueStmt(KeyValueStmt* stmt) = 0;
    virtual void visitSectionStmt(SectionStmt* stmt) = 0;
    virtual void visitDefineSectionStmt(DefineSectionStmt* stmt) = 0;
    virtual void visitIncludeStmt(IncludeStmt* stmt) = 0;
    virtual void visitQuickRegStmt(QuickRegStmt* stmt) = 0;
    virtual void visitSchemaRuleStmt(SchemaRuleStmt* stmt) = 0;
    virtual void visitSchemaSectionStmt(SchemaSectionStmt* stmt) = 0;
    virtual void visitSchemaStmt(SchemaStmt* stmt) = 0;
};

} // namespace AST
} // namespace YINI
