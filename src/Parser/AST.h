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

struct BinaryExpr : public Expr
{
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;
};

struct KeyValueStmt : public Stmt
{
    Token key;
    std::unique_ptr<Expr> value;
};

struct SectionStmt : public Stmt
{
    Token name;
    std::vector<std::unique_ptr<Stmt>> statements;
};

} // namespace AST
} // namespace YINI