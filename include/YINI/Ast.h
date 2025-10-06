#pragma once

#include <string>
#include <vector>
#include <memory>

namespace YINI
{
    // Base class for all nodes in the AST
    struct Node {
        virtual ~Node() = default;
        virtual std::string String() const = 0;
    };

    // Base class for all statements
    struct Statement : public Node {
    };

    // Base class for all expressions
    struct Expression : public Node {
    };

    // Represents an identifier
    struct Identifier : public Expression {
        std::string value;
        std::string String() const override { return value; }
    };

    // Represents an integer literal
    struct IntegerLiteral : public Expression {
        int64_t value;
        std::string String() const override { return std::to_string(value); }
    };

    struct FloatLiteral : public Expression {
        double value;
        std::string String() const override { return std::to_string(value); }
    };

    struct BooleanLiteral : public Expression {
        bool value;
        std::string String() const override { return value ? "true" : "false"; }
    };

    struct StringLiteral : public Expression {
        std::string value;
        std::string String() const override { return value; }
    };

    // Represents an array literal, e.g., [1, 2, 3]
    struct ArrayLiteral : public Expression {
        std::vector<std::shared_ptr<Expression>> elements;

        std::string String() const override {
            std::string out = "[";
            for (size_t i = 0; i < elements.size(); ++i) {
                out += elements[i]->String();
                if (i < elements.size() - 1) {
                    out += ", ";
                }
            }
            out += "]";
            return out;
        }
    };

    // Represents an infix operation, e.g., left + right
    struct InfixExpression : public Expression {
        std::shared_ptr<Expression> left;
        std::string op;
        std::shared_ptr<Expression> right;

        std::string String() const override {
            return "(" + left->String() + " " + op + " " + right->String() + ")";
        }
    };

    // Represents a key-value pair, e.g., key = 123
    struct KeyValuePair : public Statement {
        std::shared_ptr<Identifier> key;
        std::shared_ptr<Expression> value;

        std::string String() const override {
            return key->String() + " = " + (value ? value->String() : "null");
        }
    };

    // Represents a section, e.g., [Config]
    struct Section : public Statement {
        std::string name;
        std::vector<std::shared_ptr<KeyValuePair>> pairs;

        std::string String() const override {
            return "[" + name + "]";
        }
    };

    // Represents a [#define] block
    struct DefineStatement : public Statement {
        std::vector<std::shared_ptr<KeyValuePair>> pairs;

        std::string String() const override {
            return "[#define]";
        }
    };

    // Represents a macro reference, e.g., @name
    struct MacroReference : public Expression {
        std::string name;
        std::string String() const override { return "@" + name; }
    };

    // The root node of the entire YINI file
    struct Program : public Node {
        std::vector<std::shared_ptr<Statement>> statements;

        std::string String() const override {
            std::string out;
            for (const auto& s : statements) {
                out += s->String() + "\n";
            }
            return out;
        }
    };
}