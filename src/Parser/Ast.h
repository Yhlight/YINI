#pragma once

#include "Lexer/Token.h"
#include <string>
#include <vector>
#include <memory>
#include <iostream>

namespace Yini
{
// Forward declarations
struct ValueNode;
struct KeyValuePairNode;
struct SectionNode;

// Base class for all AST nodes
struct AstNode
{
    virtual ~AstNode() = default;
    virtual void serialize(std::ostream& os) const = 0;
    static std::unique_ptr<AstNode> deserialize(std::istream& is);
};

// Represents a value in a key-value pair
struct ValueNode : AstNode
{
    Token token;
    ValueNode(Token token) : token(token) {}
    void serialize(std::ostream& os) const override;
};

// Represents a single key-value pair
struct KeyValuePairNode : AstNode
{
    Token key;
    std::unique_ptr<ValueNode> value;

    KeyValuePairNode(Token key, std::unique_ptr<ValueNode> value)
        : key(key), value(std::move(value)) {}
    void serialize(std::ostream& os) const override;
};

// Represents a section with a collection of key-value pairs
struct SectionNode : AstNode
{
    Token name;
    std::vector<Token> parents;
    std::vector<std::unique_ptr<KeyValuePairNode>> pairs;

    SectionNode(Token name) : name(name) {}
    void serialize(std::ostream& os) const override;
};

} // namespace Yini