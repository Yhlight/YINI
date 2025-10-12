#pragma once

#include "Lexer/Token.h"
#include "Parser/ASTVisitor.h"
#include <memory>
#include <vector>
#include <optional>

namespace YINI
{
namespace AST
{

struct Node
{
    virtual ~Node() = default;
};

struct Expr : public Node
{
    virtual YiniVariant accept(ASTVisitor* visitor) = 0;
};

struct Stmt : public Node
{
    virtual void accept(ASTVisitor* visitor) = 0;
};

struct LiteralExpr : public Expr
{
    Token value;
    YiniVariant accept(ASTVisitor* visitor) override { return visitor->visitLiteralExpr(this); }
};

struct BoolExpr : public Expr
{
    bool value;
    YiniVariant accept(ASTVisitor* visitor) override { return visitor->visitBoolExpr(this); }
};

struct ArrayExpr : public Expr
{
    std::vector<std::unique_ptr<Expr>> elements;
    YiniVariant accept(ASTVisitor* visitor) override { return visitor->visitArrayExpr(this); }
};

struct SetExpr : public Expr
{
    std::vector<std::unique_ptr<Expr>> elements;
    YiniVariant accept(ASTVisitor* visitor) override { return visitor->visitSetExpr(this); }
};

struct MapExpr : public Expr
{
    std::vector<std::pair<Token, std::unique_ptr<Expr>>> elements;
    YiniVariant accept(ASTVisitor* visitor) override { return visitor->visitMapExpr(this); }
};

struct StructExpr : public Expr
{
    Token key;
    std::unique_ptr<Expr> value;
    YiniVariant accept(ASTVisitor* visitor) override { return visitor->visitStructExpr(this); }
};

struct ColorExpr : public Expr
{
    uint8_t r, g, b;
    YiniVariant accept(ASTVisitor* visitor) override { return visitor->visitColorExpr(this); }
};

struct CoordExpr : public Expr
{
    std::unique_ptr<Expr> x, y, z;
    YiniVariant accept(ASTVisitor* visitor) override { return visitor->visitCoordExpr(this); }
};

struct BinaryExpr : public Expr
{
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;
    YiniVariant accept(ASTVisitor* visitor) override { return visitor->visitBinaryExpr(this); }
};

struct UnaryExpr : public Expr
{
    Token op;
    std::unique_ptr<Expr> right;
    YiniVariant accept(ASTVisitor* visitor) override { return visitor->visitUnaryExpr(this); }
};

struct GroupingExpr : public Expr
{
    std::unique_ptr<Expr> expression;
    YiniVariant accept(ASTVisitor* visitor) override { return visitor->visitGroupingExpr(this); }
};

struct KeyValueStmt : public Stmt
{
    Token key;
    std::unique_ptr<Expr> value;
    void accept(ASTVisitor* visitor) override { visitor->visitKeyValueStmt(this); }
};

struct SectionStmt : public Stmt
{
    Token name;
    std::vector<Token> parent_sections;
    std::vector<std::unique_ptr<Stmt>> statements;
    void accept(ASTVisitor* visitor) override { visitor->visitSectionStmt(this); }
};

struct DefineSectionStmt : public Stmt
{
    std::vector<std::unique_ptr<KeyValueStmt>> definitions;
    void accept(ASTVisitor* visitor) override { visitor->visitDefineSectionStmt(this); }
};

struct IncludeStmt : public Stmt
{
    std::vector<std::unique_ptr<Expr>> paths;
    void accept(ASTVisitor* visitor) override { visitor->visitIncludeStmt(this, false); } // Default to not collection mode
};

struct MacroExpr : public Expr
{
    Token name;
    YiniVariant accept(ASTVisitor* visitor) override { return visitor->visitMacroExpr(this); }
};

struct CrossSectionRefExpr : public Expr
{
    Token section;
    Token key;
    YiniVariant accept(ASTVisitor* visitor) override { return visitor->visitCrossSectionRefExpr(this); }
};

struct EnvVarRefExpr : public Expr
{
    Token name;
    YiniVariant accept(ASTVisitor* visitor) override { return visitor->visitEnvVarRefExpr(this); }
};

struct DynaExpr : public Expr
{
    std::unique_ptr<Expr> expression;
    YiniVariant accept(ASTVisitor* visitor) override { return visitor->visitDynaExpr(this); }
};

struct PathExpr : public Expr
{
    std::string path;
    YiniVariant accept(ASTVisitor* visitor) override { return visitor->visitPathExpr(this); }
};

struct ListExpr : public Expr
{
    std::vector<std::unique_ptr<Expr>> elements;
    YiniVariant accept(ASTVisitor* visitor) override { return visitor->visitListExpr(this); }
};

struct QuickRegStmt : public Stmt
{
    std::unique_ptr<Expr> value;
    void accept(ASTVisitor* visitor) override { visitor->visitQuickRegStmt(this); }
};

struct SchemaRule
{
    enum class Requirement { OPTIONAL, REQUIRED };
    enum class EmptyBehavior { IGNORE, ASSIGN_DEFAULT, THROW_ERROR };

    Requirement requirement = Requirement::OPTIONAL;
    std::string type; // e.g., "int", "string", "array[int]"
    std::optional<std::string> default_value;
    std::optional<double> min;
    std::optional<double> max;
    EmptyBehavior empty_behavior = EmptyBehavior::IGNORE;
};

struct SchemaRuleStmt : public Stmt
{
    Token key;
    SchemaRule rule;
    void accept(ASTVisitor* visitor) override { visitor->visitSchemaRuleStmt(this); }
};

struct SchemaSectionStmt : public Stmt
{
    Token name;
    std::vector<std::unique_ptr<SchemaRuleStmt>> rules;
    void accept(ASTVisitor* visitor) override { visitor->visitSchemaSectionStmt(this); }
};

struct SchemaStmt : public Stmt
{
    std::vector<std::unique_ptr<SchemaSectionStmt>> sections;
    void accept(ASTVisitor* visitor) override { visitor->visitSchemaStmt(this); }
};

} // namespace AST
} // namespace YINI
