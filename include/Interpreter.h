#ifndef YINI_INTERPRETER_H
#define YINI_INTERPRETER_H

#include "AST.h"
#include "Value.h"
#include "Section.h"
#include <map>
#include <string>
#include <vector>
#include <set>
#include <stdexcept>

namespace yini
{

class InterpreterError : public std::runtime_error
{
public:
    explicit InterpreterError(const std::string& message) : std::runtime_error(message) {}
};

class Interpreter : public ASTVisitor
{
public:
    Interpreter() = default;

    // Interpret the AST and populate the configuration
    bool interpret(RootNode& root);

    // Get the results
    const std::map<std::string, Section>& getSections() const { return sections; }
    const std::map<std::string, std::shared_ptr<Value>>& getDefines() const { return defines; }
    const std::vector<std::string>& getIncludes() const { return includes; }
    std::string getLastError() const { return last_error; }

    // Visitor methods
    void visit(RootNode& node) override;
    void visit(SectionNode& node) override;
    void visit(DefineNode& node) override;
    void visit(IncludeNode& node) override;
    void visit(SchemaNode& node) override;
    void visit(KeyValuePairNode& node) override;
    void visit(LiteralNode& node) override;
    void visit(ArrayNode& node) override;
    void visit(MapNode& node) override;
    void visit(UnaryOpNode& node) override;
    void visit(BinaryOpNode& node) override;
    void visit(ReferenceNode& node) override;
    void visit(EnvVarNode& node) override;
    void visit(DynamicNode& node) override;
    void visit(FunctionCallNode& node) override;

private:
    // Evaluate an AST node to a Value
    std::shared_ptr<Value> evaluate(ASTNode& node);

    // Helpers for resolving references
    std::shared_ptr<Value> resolveValue(std::shared_ptr<Value> value, std::set<std::string>& visiting);
    std::shared_ptr<Value> resolveReference(const std::string& name, std::set<std::string>& visiting);

    // State
    std::map<std::string, Section> sections;
    std::map<std::string, std::shared_ptr<Value>> defines;
    std::vector<std::string> includes;
    std::string last_error;

    // Temporary state for evaluation
    std::shared_ptr<Value> last_value;
    std::string current_section;
};

} // namespace yini

#endif // YINI_INTERPRETER_H