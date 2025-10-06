#ifndef YINI_PARSER_ASTNODE_H
#define YINI_PARSER_ASTNODE_H

#include <string>
#include <vector>
#include <memory>
#include "../Core/Value.h"

namespace yini
{

// AST Node types
enum class ASTNodeType
{
    Section,
    KeyValue,
    Include,
    Define,
    Schema,
    Expression
};

// Base AST Node
class ASTNode
{
public:
    virtual ~ASTNode() = default;
    virtual ASTNodeType getType() const = 0;
};

// Section node [Section] : Parent1, Parent2
class SectionNode : public ASTNode
{
public:
    std::string name;
    std::vector<std::string> parents;
    std::vector<std::shared_ptr<ASTNode>> children;
    
    ASTNodeType getType() const override { return ASTNodeType::Section; }
};

// Key-value pair node
class KeyValueNode : public ASTNode
{
public:
    std::string key;
    std::shared_ptr<Value> value;
    bool is_quick_register;
    
    KeyValueNode() : is_quick_register(false) {}
    
    ASTNodeType getType() const override { return ASTNodeType::KeyValue; }
};

} // namespace yini

#endif // YINI_PARSER_ASTNODE_H
