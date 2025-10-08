#pragma once

#include "Lexer/Token.h"
#include <string>
#include <vector>
#include <memory>
#include <iostream>

namespace Yini
{

// --- Forward Declarations ---
struct Value;

// --- AST Node Base ---
struct AstNode
{
    virtual ~AstNode() = default;
    virtual void serialize(std::ostream& os) const = 0;
};


// --- Value Nodes ---
enum class ValueType {
    Identifier,
    String,
    Number,
    Bool,
    Array
};

struct Value : public AstNode {
    virtual ValueType getType() const = 0;
    virtual std::unique_ptr<Value> clone() const = 0;
};

struct IdentifierValue : public Value {
    Token token;
    explicit IdentifierValue(Token t) : token(std::move(t)) {}
    ValueType getType() const override { return ValueType::Identifier; }
    void serialize(std::ostream& os) const override;
    std::unique_ptr<Value> clone() const override { return std::make_unique<IdentifierValue>(*this); }
};

struct StringValue : public Value {
    std::string value;
    explicit StringValue(std::string v) : value(std::move(v)) {}
    ValueType getType() const override { return ValueType::String; }
    void serialize(std::ostream& os) const override;
    std::unique_ptr<Value> clone() const override { return std::make_unique<StringValue>(*this); }
};

struct NumberValue : public Value {
    double value;
    explicit NumberValue(double v) : value(v) {}
    ValueType getType() const override { return ValueType::Number; }
    void serialize(std::ostream& os) const override;
    std::unique_ptr<Value> clone() const override { return std::make_unique<NumberValue>(*this); }
};

struct BoolValue : public Value {
    bool value;
    explicit BoolValue(bool v) : value(v) {}
    ValueType getType() const override { return ValueType::Bool; }
    void serialize(std::ostream& os) const override;
    std::unique_ptr<Value> clone() const override { return std::make_unique<BoolValue>(*this); }
};

struct ArrayValue : public Value {
    std::vector<std::unique_ptr<Value>> elements;
    ValueType getType() const override { return ValueType::Array; }
    void serialize(std::ostream& os) const override;
    std::unique_ptr<Value> clone() const override {
        auto new_array = std::make_unique<ArrayValue>();
        for (const auto& elem : elements) {
            new_array->elements.push_back(elem->clone());
        }
        return new_array;
    }
};


// --- Structural Nodes ---
struct KeyValuePairNode : public AstNode
{
    Token key;
    std::unique_ptr<Value> value;

    KeyValuePairNode(Token key, std::unique_ptr<Value> value)
        : key(std::move(key)), value(std::move(value)) {}
    void serialize(std::ostream& os) const override;
};

struct SectionNode : public AstNode
{
    Token name;
    std::vector<Token> parents;
    std::vector<std::unique_ptr<KeyValuePairNode>> pairs;

    SectionNode(Token name) : name(std::move(name)) {}
    void serialize(std::ostream& os) const override;
};

// --- Deserialization Forward Declarations ---
std::unique_ptr<Value> deserializeValue(std::istream& is);
std::unique_ptr<KeyValuePairNode> deserializeKeyValuePair(std::istream& is);
std::unique_ptr<SectionNode> deserializeSection(std::istream& is);

} // namespace Yini