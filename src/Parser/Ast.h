#pragma once

#include "../Lexer/Token.h"
#include <vector>
#include <string>
#include <memory>

namespace YINI
{
    // Forward declarations
    struct LiteralExpr;
    struct BinaryExpr;
    struct GroupingExpr;
    struct SectionStmt;
    struct KeyValueStmt;
    struct ArrayExpr;
    struct CallExpr;
    struct RegisterStmt;

    // Visitor pattern for expressions
    class ExprVisitor
    {
    public:
        virtual ~ExprVisitor() = default;
        virtual void visit(const LiteralExpr& expr) = 0;
        virtual void visit(const BinaryExpr& expr) = 0;
        virtual void visit(const GroupingExpr& expr) = 0;
        virtual void visit(const ArrayExpr& expr) = 0;
        virtual void visit(const CallExpr& expr) = 0;
    };

    // Base class for all expression nodes
    struct Expr
    {
        virtual ~Expr() = default;
        virtual void accept(ExprVisitor& visitor) const = 0;
    };

    // Literal expressions (string, int, float, bool)
    struct LiteralExpr : Expr
    {
        LiteralExpr(Token token) : token(token) {}
        void accept(ExprVisitor& visitor) const override { visitor.visit(*this); }
        Token token;
    };

    // Binary expressions for arithmetic operations
    struct BinaryExpr : Expr
    {
        BinaryExpr(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right)
            : left(std::move(left)), op(op), right(std::move(right)) {}
        void accept(ExprVisitor& visitor) const override { visitor.visit(*this); }
        std::unique_ptr<Expr> left;
        Token op;
        std::unique_ptr<Expr> right;
    };

    // Grouping expressions for controlling precedence
    struct GroupingExpr : Expr
    {
        GroupingExpr(std::unique_ptr<Expr> expression) : expression(std::move(expression)) {}
        void accept(ExprVisitor& visitor) const override { visitor.visit(*this); }
        std::unique_ptr<Expr> expression;
    };

    // Array expression
    struct ArrayExpr : Expr
    {
        ArrayExpr(std::vector<std::unique_ptr<Expr>> elements)
            : elements(std::move(elements)) {}
        void accept(ExprVisitor& visitor) const override { visitor.visit(*this); }
        std::vector<std::unique_ptr<Expr>> elements;
    };

    // Call expression for Coord, Color, etc.
    struct CallExpr : Expr
    {
        CallExpr(std::unique_ptr<Expr> callee, Token paren, std::vector<std::unique_ptr<Expr>> arguments)
            : callee(std::move(callee)), paren(paren), arguments(std::move(arguments)) {}
        void accept(ExprVisitor& visitor) const override { visitor.visit(*this); }
        std::unique_ptr<Expr> callee;
        Token paren; // To store the location of the '(', for error reporting
        std::vector<std::unique_ptr<Expr>> arguments;
    };

    // Visitor pattern for statements
    class StmtVisitor
    {
    public:
        virtual ~StmtVisitor() = default;
        virtual void visit(const SectionStmt& stmt) = 0;
        virtual void visit(const KeyValueStmt& stmt) = 0;
        virtual void visit(const RegisterStmt& stmt) = 0;
    };

    // Base class for all statement nodes
    struct Stmt
    {
        virtual ~Stmt() = default;
        virtual void accept(StmtVisitor& visitor) const = 0;
    };

    // Section statement
    struct SectionStmt : Stmt
    {
        SectionStmt(Token name, std::vector<Token> inheritance)
            : name(name), inheritance(inheritance) {}
        void accept(StmtVisitor& visitor) const override { visitor.visit(*this); }
        Token name;
        std::vector<Token> inheritance;
        std::vector<std::unique_ptr<Stmt>> statements;
    };

    // Key-value pair statement
    struct KeyValueStmt : Stmt
    {
        KeyValueStmt(Token key, std::unique_ptr<Expr> value)
            : key(key), value(std::move(value)) {}
        void accept(StmtVisitor& visitor) const override { visitor.visit(*this); }
        Token key;
        std::unique_ptr<Expr> value;
    };

    // Register statement for +=
    struct RegisterStmt : Stmt
    {
        RegisterStmt(Token op, std::unique_ptr<Expr> value)
            : op(op), value(std::move(value)) {}
        void accept(StmtVisitor& visitor) const override { visitor.visit(*this); }
        Token op; // The += token
        std::unique_ptr<Expr> value;
    };
}