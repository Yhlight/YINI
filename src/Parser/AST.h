#pragma once

#include <string>
#include <vector>
#include <memory>
#include "../Lexer/Token.h"

namespace YINI
{
    namespace AST
    {
        // Base class for all nodes in the AST
        struct Node
        {
            virtual ~Node() = default;
            virtual std::string toString() const = 0;
        };

        // Base class for all expression nodes
        struct Expression : public Node
        {
        };

        // Represents an identifier, e.g., a key or section name
        struct Identifier : public Expression
        {
            Token token; // The IDENTIFIER token
            std::string value;
            std::string toString() const override { return value; }
        };

        // --- Literal Expressions ---

        struct StringLiteral : public Expression
        {
            Token token;
            std::string value;
            std::string toString() const override { return "\"" + value + "\""; }
        };

        struct IntegerLiteral : public Expression
        {
            Token token;
            int64_t value;
            std::string toString() const override { return std::to_string(value); }
        };

        struct FloatLiteral : public Expression
        {
            Token token;
            double value;
            std::string toString() const override { return std::to_string(value); }
        };

        struct BooleanLiteral : public Expression
        {
            Token token;
            bool value;
            std::string toString() const override { return value ? "true" : "false"; }
        };

        struct MacroReference : public Expression
        {
            Token token; // The '@' token
            std::unique_ptr<Identifier> name;
            std::string toString() const override { return "@" + name->toString(); }
        };

        struct InfixExpression : public Expression
        {
            std::unique_ptr<Expression> left;
            Token token; // The operator token, e.g., +
            std::unique_ptr<Expression> right;
            std::string toString() const override
            {
                return "(" + left->toString() + " " + token.literal + " " + right->toString() + ")";
            }
        };

        struct DynaExpression : public Expression
        {
            Token token; // The 'Dyna' token
            std::unique_ptr<Expression> wrapped_expression;
            std::string toString() const override
            {
                return "Dyna(" + wrapped_expression->toString() + ")";
            }
        };

        // Base class for all statement nodes
        struct Statement : public Node
        {
        };

        // The root node of the AST. A YINI file is a program consisting of statements.
        struct Program : public Node
        {
            std::vector<std::unique_ptr<Statement>> statements;
            std::string toString() const override
            {
                std::string out;
                for (const auto& stmt : statements)
                {
                    out += stmt->toString();
                }
                return out;
            }
        };

        // Represents a key-value pair, e.g., key = value
        struct KeyValuePair : public Statement
        {
            std::unique_ptr<Identifier> key;
            std::unique_ptr<Expression> value;
            std::string toString() const override
            {
                std::string out = key->toString();
                out += " = ";
                if (value)
                {
                    out += value->toString();
                }
                else
                {
                    out += "(null)"; // Placeholder for now
                }
                return out;
            }
        };

        // Represents a section, e.g., [SectionName]
        struct Section : public Statement
        {
            std::unique_ptr<Identifier> name;
            std::vector<std::unique_ptr<Identifier>> inheritance_list;
            std::vector<std::unique_ptr<Statement>> statements;
            std::string toString() const override
            {
                std::string out = "[" + name->toString() + "]\n";
                for (const auto& stmt : statements)
                {
                    out += "  " + stmt->toString() + "\n";
                }
                return out;
            }
        };

    } // namespace AST
} // namespace YINI
