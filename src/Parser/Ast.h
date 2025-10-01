#pragma once

#include "Lexer/Token.h"
#include <vector>
#include <memory>

namespace YINI
{
    // Forward declarations for the visitor pattern
    struct KeyValue;
    struct Section;

    class AstVisitor
    {
    public:
        virtual void visit(const KeyValue& stmt) = 0;
        virtual void visit(const Section& stmt) = 0;
    };

    class AstNode
    {
    public:
        virtual ~AstNode() = default;
        virtual void accept(AstVisitor& visitor) const = 0;
    };

    struct KeyValue : public AstNode
    {
        KeyValue(Token key, Token value) : key(key), value(value) {}

        void accept(AstVisitor& visitor) const override
        {
            visitor.visit(*this);
        }

        Token key;
        Token value;
    };

    struct Section : public AstNode
    {
        Section(Token name, std::vector<std::unique_ptr<KeyValue>> values)
            : name(name), values(std::move(values)) {}

        void accept(AstVisitor& visitor) const override
        {
            visitor.visit(*this);
        }

        Token name;
        std::vector<std::unique_ptr<KeyValue>> values;
    };
}