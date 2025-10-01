#pragma once

#include "Lexer/Token.h"
#include <vector>
#include <memory>
#include <any>

namespace YINI
{
    // Forward declarations for the visitor pattern
    struct Literal;

    // Visitor for expressions
    class ExprVisitor
    {
    public:
        virtual std::any visit(const Literal& expr) = 0;
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

        std::any accept(ExprVisitor& visitor) const override
        {
            return visitor.visit(*this);
        }

        std::any value;
    };

    // Forward declarations for statement visitors
    struct KeyValue;
    struct Section;

    // Visitor for statements
    class StmtVisitor
    {
    public:
        virtual void visit(const KeyValue& stmt) = 0;
        virtual void visit(const Section& stmt) = 0;
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

        void accept(StmtVisitor& visitor) const override
        {
            visitor.visit(*this);
        }

        Token key;
        std::unique_ptr<Expr> value;
    };

    struct Section : public Stmt
    {
        Section(Token name, std::vector<std::unique_ptr<KeyValue>> values)
            : name(std::move(name)), values(std::move(values)) {}

        void accept(StmtVisitor& visitor) const override
        {
            visitor.visit(*this);
        }

        Token name;
        std::vector<std::unique_ptr<KeyValue>> values;
    };
}