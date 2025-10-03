#pragma once

#include "Lexer/Token.h"
#include <vector>
#include "Core/YiniValue.h"
#include <map>
#include <memory>
#include <vector>

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
    struct Variable;
    struct EnvVariable;
    struct XRef;

    // Visitor for expressions
    class ExprVisitor
    {
    public:
        virtual YiniValue visit(const Literal& expr) = 0;
        virtual YiniValue visit(const Unary& expr) = 0;
        virtual YiniValue visit(const Binary& expr) = 0;
        virtual YiniValue visit(const Grouping& expr) = 0;
        virtual YiniValue visit(const Array& expr) = 0;
        virtual YiniValue visit(const Set& expr) = 0;
        virtual YiniValue visit(const Map& expr) = 0;
        virtual YiniValue visit(const Call& expr) = 0;
        virtual YiniValue visit(const Variable& expr) = 0;
        virtual YiniValue visit(const EnvVariable& expr) = 0;
        virtual YiniValue visit(const XRef& expr) = 0;
        virtual ~ExprVisitor() = default;
    };

    // Base class for expressions
    class Expr
    {
    public:
        virtual ~Expr() = default;
        virtual YiniValue accept(ExprVisitor& visitor) const = 0;
    };

    // Literal expression node
    struct Literal : public Expr
    {
        Literal(YiniValue value) : value(std::move(value)) {}
        YiniValue accept(ExprVisitor& visitor) const override { return visitor.visit(*this); }
        YiniValue value;
    };

    // Unary expression node
    struct Unary : public Expr
    {
        Unary(Token op, std::unique_ptr<Expr> right) : op(std::move(op)), right(std::move(right)) {}
        YiniValue accept(ExprVisitor& visitor) const override { return visitor.visit(*this); }
        Token op;
        std::unique_ptr<Expr> right;
    };

    // Binary expression node
    struct Binary : public Expr
    {
        Binary(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right)
            : left(std::move(left)), op(std::move(op)), right(std::move(right)) {}
        YiniValue accept(ExprVisitor& visitor) const override { return visitor.visit(*this); }
        std::unique_ptr<Expr> left;
        Token op;
        std::unique_ptr<Expr> right;
    };

    // Grouping expression node
    struct Grouping : public Expr
    {
        Grouping(std::unique_ptr<Expr> expression) : expression(std::move(expression)) {}
        YiniValue accept(ExprVisitor& visitor) const override { return visitor.visit(*this); }
        std::unique_ptr<Expr> expression;
    };

    // Array expression node
    struct Array : public Expr
    {
        Array(std::vector<std::unique_ptr<Expr>> elements)
            : elements(std::move(elements)) {}
        YiniValue accept(ExprVisitor& visitor) const override { return visitor.visit(*this); }
        std::vector<std::unique_ptr<Expr>> elements;
    };

    // Set expression node
    struct Set : public Expr
    {
        Set(std::vector<std::unique_ptr<Expr>> elements)
            : elements(std::move(elements)) {}
        YiniValue accept(ExprVisitor& visitor) const override { return visitor.visit(*this); }
        std::vector<std::unique_ptr<Expr>> elements;
    };

    // Map expression node
    struct Map : public Expr
    {
        Map(Token brace, std::vector<std::pair<std::unique_ptr<Expr>, std::unique_ptr<Expr>>> pairs)
            : brace(std::move(brace)), pairs(std::move(pairs)) {}
        YiniValue accept(ExprVisitor& visitor) const override { return visitor.visit(*this); }
        Token brace;
        std::vector<std::pair<std::unique_ptr<Expr>, std::unique_ptr<Expr>>> pairs;
    };

    // Call expression node
    struct Call : public Expr
    {
        Call(std::unique_ptr<Expr> callee, Token paren, std::vector<std::unique_ptr<Expr>> arguments)
            : callee(std::move(callee)), paren(std::move(paren)), arguments(std::move(arguments)) {}
        YiniValue accept(ExprVisitor& visitor) const override { return visitor.visit(*this); }
        std::unique_ptr<Expr> callee;
        Token paren; // The closing ')'
        std::vector<std::unique_ptr<Expr>> arguments;
    };

    // Variable expression node
    struct Variable : public Expr
    {
        Variable(Token name) : name(std::move(name)) {}
        YiniValue accept(ExprVisitor& visitor) const override { return visitor.visit(*this); }
        Token name;
    };

    // Environment variable expression node, e.g., ${VAR_NAME:default_value}
    struct EnvVariable : public Expr
    {
        EnvVariable(Token name, std::unique_ptr<Expr> default_value)
            : name(std::move(name)), default_value(std::move(default_value)) {}
        YiniValue accept(ExprVisitor& visitor) const override { return visitor.visit(*this); }
        Token name; // The token representing the variable's name
        std::unique_ptr<Expr> default_value; // The expression for the default value, can be nullptr
    };

    // Environment variable expression node, e.g., ${VAR_NAME:default_value}
    struct XRef : public Expr
    {
        XRef(Token section, Token key)
            : section(std::move(section)), key(std::move(key)) {}
        YiniValue accept(ExprVisitor& visitor) const override { return visitor.visit(*this); }
        Token section;
        Token key;
    };

    // Forward declarations for statement visitors
    struct KeyValue;
    struct Section;
    struct Register;
    struct Define;
    struct Include;
    struct Schema;

    // Visitor for statements
    class StmtVisitor
    {
    public:
        virtual void visit(const KeyValue& stmt) = 0;
        virtual void visit(const Section& stmt) = 0;
        virtual void visit(const Register& stmt) = 0;
        virtual void visit(const Define& stmt) = 0;
        virtual void visit(const Include& stmt) = 0;
        virtual void visit(const Schema& stmt) = 0;
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
        int value_line = 0;
        int value_column = 0;
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

    struct Define : public Stmt
    {
        Define(std::vector<std::unique_ptr<KeyValue>> values) : values(std::move(values)) {}
        void accept(StmtVisitor& visitor) const override { visitor.visit(*this); }
        std::vector<std::unique_ptr<KeyValue>> values;
    };

    struct Include : public Stmt
    {
        Include(std::vector<std::unique_ptr<Expr>> files) : files(std::move(files)) {}
        void accept(StmtVisitor& visitor) const override { visitor.visit(*this); }
        std::vector<std::unique_ptr<Expr>> files;
    };

    // Schema statement node, representing a [#schema] block
    struct Schema : public Stmt
    {
        Schema(std::vector<std::unique_ptr<Section>> sections)
            : sections(std::move(sections)) {}
        void accept(StmtVisitor& visitor) const override { visitor.visit(*this); }
        std::vector<std::unique_ptr<Section>> sections;
    };
}