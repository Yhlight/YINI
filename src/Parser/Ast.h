#pragma once

#include "Lexer/Token.h"
#include <vector>
#include <memory>
#include <any>
#include <map>

namespace YINI
{
    // Forward declarations for expressions
    struct Literal;
    struct Unary;
    struct Binary;
    struct Grouping;
    struct Array;
    struct Set;
    struct Map;
    struct Call;

    // Visitor for expressions
    class ExprVisitor
    {
    public:
        virtual std::any visit(const Literal& expr) = 0;
        virtual std::any visit(const Unary& expr) = 0;
        virtual std::any visit(const Binary& expr) = 0;
        virtual std::any visit(const Grouping& expr) = 0;
        virtual std::any visit(const Array& expr) = 0;
        virtual std::any visit(const Set& expr) = 0;
        virtual std::any visit(const Map& expr) = 0;
        virtual std::any visit(const Call& expr) = 0;
        virtual ~ExprVisitor() = default;
    };

    // Base class for expressions
    class Expr
    {
    public:
        virtual ~Expr() = default;
        virtual std::any accept(ExprVisitor& visitor) const = 0;
    };

    // Literal expression node
    struct Literal : public Expr
    {
        Literal(std::any value) : value(std::move(value)) {}
        std::any accept(ExprVisitor& visitor) const override { return visitor.visit(*this); }
        std::any value;
    };

    // Unary expression node
    struct Unary : public Expr
    {
        Unary(Token op, std::unique_ptr<Expr> right) : op(std::move(op)), right(std::move(right)) {}
        std::any accept(ExprVisitor& visitor) const override { return visitor.visit(*this); }
        Token op;
        std::unique_ptr<Expr> right;
    };

    // Binary expression node
    struct Binary : public Expr
    {
        Binary(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right)
            : left(std::move(left)), op(std::move(op)), right(std::move(right)) {}
        std::any accept(ExprVisitor& visitor) const override { return visitor.visit(*this); }
        std::unique_ptr<Expr> left;
        Token op;
        std::unique_ptr<Expr> right;
    };

    // Grouping expression node
    struct Grouping : public Expr
    {
        Grouping(std::unique_ptr<Expr> expression) : expression(std::move(expression)) {}
        std::any accept(ExprVisitor& visitor) const override { return visitor.visit(*this); }
        std::unique_ptr<Expr> expression;
    };

    // Array expression node
    struct Array : public Expr
    {
        Array(std::vector<std::unique_ptr<Expr>> elements)
            : elements(std::move(elements)) {}
        std::any accept(ExprVisitor& visitor) const override { return visitor.visit(*this); }
        std::vector<std::unique_ptr<Expr>> elements;
    };

    // Set expression node
    struct Set : public Expr
    {
        Set(std::vector<std::unique_ptr<Expr>> elements)
            : elements(std::move(elements)) {}
        std::any accept(ExprVisitor& visitor) const override { return visitor.visit(*this); }
        std::vector<std::unique_ptr<Expr>> elements;
    };

    // Map expression node
    struct Map : public Expr
    {
        Map(std::vector<std::pair<std::unique_ptr<Expr>, std::unique_ptr<Expr>>> pairs)
            : pairs(std::move(pairs)) {}
        std::any accept(ExprVisitor& visitor) const override { return visitor.visit(*this); }
        std::vector<std::pair<std::unique_ptr<Expr>, std::unique_ptr<Expr>>> pairs;
    };

    // Call expression node
    struct Call : public Expr
    {
        Call(std::unique_ptr<Expr> callee, Token paren, std::vector<std::unique_ptr<Expr>> arguments)
            : callee(std::move(callee)), paren(std::move(paren)), arguments(std::move(arguments)) {}
        std::any accept(ExprVisitor& visitor) const override { return visitor.visit(*this); }
        std::unique_ptr<Expr> callee;
        Token paren; // The closing ')'
        std::vector<std::unique_ptr<Expr>> arguments;
    };

    // Forward declarations for statement visitors
    struct KeyValue;
    struct Section;
    struct Register;

    // Visitor for statements
    class StmtVisitor
    {
    public:
        virtual void visit(const KeyValue& stmt) = 0;
        virtual void visit(const Section& stmt) = 0;
        virtual void visit(const Register& stmt) = 0;
        virtual ~StmtVisitor() = default;
    };

    // Base class for statements
    class Stmt
    {
    public:
        virtual ~Stmt() = default;
        virtual void accept(StmtVisitor& visitor) const = 0;
    };

    struct KeyValue : public Stmt
    {
        KeyValue(Token key, std::unique_ptr<Expr> value) : key(std::move(key)), value(std::move(value)) {}
        void accept(StmtVisitor& visitor) const override { visitor.visit(*this); }
        Token key;
        std::unique_ptr<Expr> value;
    };

    struct Section : public Stmt
    {
        Section(Token name, std::vector<Token> parents, std::vector<std::unique_ptr<Stmt>> statements)
            : name(std::move(name)), parents(std::move(parents)), statements(std::move(statements)) {}
        void accept(StmtVisitor& visitor) const override { visitor.visit(*this); }
        Token name;
        std::vector<Token> parents;
        std::vector<std::unique_ptr<Stmt>> statements;
    };

    struct Register : public Stmt
    {
        Register(std::unique_ptr<Expr> value) : value(std::move(value)) {}
        void accept(StmtVisitor& visitor) const override { visitor.visit(*this); }
        std::unique_ptr<Expr> value;
    };
}