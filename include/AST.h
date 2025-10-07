#ifndef YINI_AST_H
#define YINI_AST_H

#include "Token.h"
#include "Value.h"
#include <vector>
#include <string>
#include <memory>

namespace yini
{

// Forward declarations for the visitor pattern
struct RootNode;
struct SectionNode;
struct DefineNode;
struct IncludeNode;
struct SchemaNode;
struct KeyValuePairNode;
struct LiteralNode;
struct ArrayNode;
struct MapNode;
struct UnaryOpNode;
struct BinaryOpNode;
struct ReferenceNode;
struct EnvVarNode;
struct DynamicNode;
struct FunctionCallNode;

// Visitor interface
struct ASTVisitor
{
    virtual ~ASTVisitor() = default;
    virtual void visit(RootNode& node) = 0;
    virtual void visit(SectionNode& node) = 0;
    virtual void visit(DefineNode& node) = 0;
    virtual void visit(IncludeNode& node) = 0;
    virtual void visit(SchemaNode& node) = 0;
    virtual void visit(KeyValuePairNode& node) = 0;
    virtual void visit(LiteralNode& node) = 0;
    virtual void visit(ArrayNode& node) = 0;
    virtual void visit(MapNode& node) = 0;
    virtual void visit(UnaryOpNode& node) = 0;
    virtual void visit(BinaryOpNode& node) = 0;
    virtual void visit(ReferenceNode& node) = 0;
    virtual void visit(EnvVarNode& node) = 0;
    virtual void visit(DynamicNode& node) = 0;
    virtual void visit(FunctionCallNode& node) = 0;
};

// Base class for all AST nodes
struct ASTNode
{
    virtual ~ASTNode() = default;
    virtual void accept(ASTVisitor& visitor) = 0;
};

// Represents the root of a YINI file
struct RootNode : ASTNode
{
    std::vector<std::shared_ptr<ASTNode>> children;
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

// Represents a [Section]
struct SectionNode : ASTNode
{
    std::string name;
    std::vector<std::string> inherited_sections;
    std::vector<std::shared_ptr<KeyValuePairNode>> children;
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

// Represents a [#define] section
struct DefineNode : ASTNode
{
    std::vector<std::shared_ptr<KeyValuePairNode>> definitions;
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

// Represents a [#include] section
struct IncludeNode : ASTNode
{
    std::vector<std::string> files;
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

// Represents a [#schema] section (structure only for now)
struct SchemaNode : ASTNode
{
    // Simplified for now, can be expanded later
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

// Represents a key = value pair
struct KeyValuePairNode : ASTNode
{
    std::string key;
    std::shared_ptr<ASTNode> value;
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

// Represents a literal value (int, float, string, bool)
struct LiteralNode : ASTNode
{
    std::shared_ptr<Value> value;
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

// Represents an array [...]
struct ArrayNode : ASTNode
{
    std::vector<std::shared_ptr<ASTNode>> elements;
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

// Represents a map {...}
struct MapNode : ASTNode
{
    std::vector<std::shared_ptr<KeyValuePairNode>> pairs;
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

// Represents a unary operation (e.g., -5)
struct UnaryOpNode : ASTNode
{
    Token op;
    std::shared_ptr<ASTNode> right;
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

// Represents a binary operation (e.g., 1 + 2)
struct BinaryOpNode : ASTNode
{
    std::shared_ptr<ASTNode> left;
    Token op;
    std::shared_ptr<ASTNode> right;
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

// Represents a reference (@define or @{section.key})
struct ReferenceNode : ASTNode
{
    std::string name;
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

// Represents an environment variable (${VAR})
struct EnvVarNode : ASTNode
{
    std::string name;
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

// Represents a dynamic value (Dyna(...))
struct DynamicNode : ASTNode
{
    std::shared_ptr<ASTNode> value;
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

// Represents a function-style call (e.g., Color(...))
struct FunctionCallNode : ASTNode
{
    std::string callee_name;
    std::vector<std::shared_ptr<ASTNode>> arguments;
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

} // namespace yini

#endif // YINI_AST_H