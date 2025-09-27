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

    // Represents a key-value pair, e.g., key = "value"
    struct KeyValuePairStatement : public Statement {
        Token token; // The key's token (an Identifier)
        Token value; // The value's token

        std::string tokenLiteral() const override { return token.literal; }
    };
}