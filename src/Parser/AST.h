#pragma once

#include <string>
#include <vector>
#include <memory>
#include "Lexer/Token.h"

namespace YINI
{
    // Base class for all nodes in the AST
    struct Node {
        virtual ~Node() = default;
        virtual std::string tokenLiteral() const = 0;
    };

    // Base class for all statement nodes
    struct Statement : public Node {
        // A dummy method to satisfy the pure virtual function in Node
        std::string tokenLiteral() const override { return ""; }
    };

    // The root node of every AST our parser produces.
    // A YINI program is just a sequence of statements.
    struct Program : public Node {
        std::vector<std::unique_ptr<Statement>> statements;

        std::string tokenLiteral() const override {
            if (!statements.empty()) {
                return statements[0]->tokenLiteral();
            }
            return "";
        }
    };

    // Represents a section declaration, e.g., [SectionName]
    struct SectionStatement : public Statement {
        Token token; // The '[' token
        std::string name;

        std::string tokenLiteral() const override { return token.literal; }
    };

    // Base class for all expression nodes
    struct Expression : public Node {
        // A dummy method to satisfy the pure virtual function in Node
        std::string tokenLiteral() const override { return ""; }
    };

    // Represents a key-value pair, e.g., key = "value"
    struct KeyValuePairStatement : public Statement {
        Token token; // The key's token (an Identifier)
        std::unique_ptr<Expression> value; // The expression on the right side of '='

        std::string tokenLiteral() const override { return token.literal; }
    };

    // Expression nodes for literal values
    struct StringLiteral : public Expression {
        Token token;
        std::string value;
        std::string tokenLiteral() const override { return token.literal; }
    };

    struct IntegerLiteral : public Expression {
        Token token;
        int64_t value;
        std::string tokenLiteral() const override { return token.literal; }
    };

    struct FloatLiteral : public Expression {
        Token token;
        double value;
        std::string tokenLiteral() const override { return token.literal; }
    };

    struct BooleanLiteral : public Expression {
        Token token;
        bool value;
        std::string tokenLiteral() const override { return token.literal; }
    };

    struct InfixExpression : public Expression {
        std::unique_ptr<Expression> left;
        Token operator_token; // The operator token, e.g. +
        std::unique_ptr<Expression> right;
        std::string tokenLiteral() const override { return operator_token.literal; }
    };

    struct PrefixExpression : public Expression {
        Token operator_token; // The prefix token, e.g. -
        std::unique_ptr<Expression> right;
        std::string tokenLiteral() const override { return operator_token.literal; }
    };

    // Represents a quick registration, e.g., += value
    struct QuickRegisterStatement : public Statement {
        Token token; // The '+=' token
        std::unique_ptr<Expression> value;

        std::string tokenLiteral() const override { return token.literal; }
    };
}