#pragma once

#include "Lexer/Token.h"
#include <memory>
#include <vector>

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
};

struct Stmt : public Node
{
};

struct LiteralExpr : public Expr
{
    Token value;
};

struct BoolExpr : public Expr
{
    bool value;
};

struct ArrayExpr : public Expr
{
    std::vector<std::unique_ptr<Expr>> elements;
};

struct SetExpr : public Expr
{
    std::vector<std::unique_ptr<Expr>> elements;
};

struct MapExpr : public Expr
{
    std::vector<std::pair<Token, std::unique_ptr<Expr>>> elements;
};

struct ColorExpr : public Expr
{
    uint8_t r, g, b;
};

struct CoordExpr : public Expr
{
    std::unique_ptr<Expr> x, y, z;
};

struct BinaryExpr : public Expr
{
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;
};

struct GroupingExpr : public Expr
{
    std::unique_ptr<Expr> expression;
};

struct KeyValueStmt : public Stmt
{
    Token key;
    std::unique_ptr<Expr> value;
};

struct SectionStmt : public Stmt
{
    Token name;
    std::vector<Token> parent_sections;
    std::vector<std::unique_ptr<Stmt>> statements;
};

struct DefineSectionStmt : public Stmt
{
    std::vector<std::unique_ptr<KeyValueStmt>> definitions;
};

struct IncludeStmt : public Stmt
{
    std::vector<std::unique_ptr<Expr>> paths;
};

struct MacroExpr : public Expr
{
    Token name;
};

struct CrossSectionRefExpr : public Expr
{
    Token section;
    Token key;
};

struct EnvVarRefExpr : public Expr
{
    Token name;
};

struct DynaExpr : public Expr
{
    std::unique_ptr<Expr> expression;
};

struct PathExpr : public Expr
{
    std::string path;
};

struct ListExpr : public Expr
{
    std::vector<std::unique_ptr<Expr>> elements;
};

struct QuickRegStmt : public Stmt
{
    std::unique_ptr<Expr> value;
};

struct SchemaRuleStmt : public Stmt
{
    Token key;
    Token rules;
};

struct SchemaSectionStmt : public Stmt
{
    Token name;
    std::vector<std::unique_ptr<SchemaRuleStmt>> rules;
};

struct SchemaStmt : public Stmt
{
    std::vector<std::unique_ptr<SchemaSectionStmt>> sections;
};

} // namespace AST
} // namespace YINI