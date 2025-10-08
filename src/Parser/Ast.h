#pragma once

#include "Lexer/Token.h"
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <map>
#include <cstdint>

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
    Array,
    Set,
    Map,
    Color,
    Coord,
    Path,
    Reference
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
    std::unique_ptr<Value> clone() const override;
};

struct SetValue : public Value {
    std::vector<std::unique_ptr<Value>> elements;
    ValueType getType() const override { return ValueType::Set; }
    void serialize(std::ostream& os) const override;
    std::unique_ptr<Value> clone() const override;
};

struct MapValue : public Value {
    std::map<std::string, std::unique_ptr<Value>> elements;
    ValueType getType() const override { return ValueType::Map; }
    void serialize(std::ostream& os) const override;
    std::unique_ptr<Value> clone() const override;
};

struct ColorValue : public Value {
    uint8_t r, g, b, a;
    explicit ColorValue(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) : r(r), g(g), b(b), a(a) {}
    ValueType getType() const override { return ValueType::Color; }
    void serialize(std::ostream& os) const override;
    std::unique_ptr<Value> clone() const override { return std::make_unique<ColorValue>(*this); }
};

struct CoordValue : public Value {
    double x, y, z;
    bool has_z;
    explicit CoordValue(double x, double y, double z, bool has_z = true) : x(x), y(y), z(z), has_z(has_z) {}
    ValueType getType() const override { return ValueType::Coord; }
    void serialize(std::ostream& os) const override;
    std::unique_ptr<Value> clone() const override { return std::make_unique<CoordValue>(*this); }
};

struct PathValue : public Value {
    std::string path;
    explicit PathValue(std::string p) : path(std::move(p)) {}
    ValueType getType() const override { return ValueType::Path; }
    void serialize(std::ostream& os) const override;
    std::unique_ptr<Value> clone() const override { return std::make_unique<PathValue>(*this); }
};

struct ReferenceValue : public Value {
    Token token;
    explicit ReferenceValue(Token t) : token(std::move(t)) {}
    ValueType getType() const override { return ValueType::Reference; }
    void serialize(std::ostream& os) const override;
    std::unique_ptr<Value> clone() const override { return std::make_unique<ReferenceValue>(*this); }
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

enum class SpecialSectionType {
    None,
    Define,
    Include
};

struct SectionNode : public AstNode
{
    Token name;
    SpecialSectionType special_type = SpecialSectionType::None;
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