#ifndef YINI_RUNTIME_H
#define YINI_RUNTIME_H

#include "../Parser/Ast.h"
#include "../Common/Error.h"
#include "Value.h"
#include <map>
#include <string>
#include <memory>
#include <set>

namespace Yini
{
    // Type aliases for clarity
    using SectionData = std::map<std::string, std::shared_ptr<Value>>;
    using SectionAst = std::map<std::string, Ast::Expression*>;

    class YiniRuntime
    {
    public:
        YiniRuntime() = default;

        // Populates the runtime with the AST, but does not evaluate.
        void load(Ast::YiniDocument* doc);

        // Public API to get values. Triggers lazy evaluation.
        std::shared_ptr<Value> getValue(const std::string& sectionName, const std::string& key);

        // Public API to set values.
        bool setValue(const std::string& sectionName, const std::string& key, std::shared_ptr<Value> value);

        const std::vector<YiniError>& getErrors() const;

        // For debugging: dump the whole state
        std::string dump() const;

        // Serialization
        bool serialize(const std::string& filepath);
        bool deserialize(const std::string& filepath);

    private:
        // Evaluation function for a single expression
        std::shared_ptr<Value> evaluateExpression(Ast::Expression* expr, const std::string& sectionScope);

        // Visitor methods for each AST node type
        std::shared_ptr<Value> visit(Ast::Node* node, const std::string& sectionScope) const;
        std::shared_ptr<Value> visit(Ast::Expression* node, const std::string& sectionScope) const;
        std::shared_ptr<Value> visit(Ast::IntegerLiteral* node) const;
        std::shared_ptr<Value> visit(Ast::FloatLiteral* node) const;
        std::shared_ptr<Value> visit(Ast::BooleanLiteral* node) const;
        std::shared_ptr<Value> visit(Ast::StringLiteral* node) const;
        std::shared_ptr<Value> visit(Ast::ColorLiteral* node) const;
        std::shared_ptr<Value> visit(Ast::InfixExpression* node, const std::string& sectionScope) const;
        std::shared_ptr<Value> visit(Ast::Identifier* node, const std::string& sectionScope) const;
        std::shared_ptr<Value> visit(Ast::MacroReference* node) const;
        std::shared_ptr<Value> visit(Ast::ArrayLiteral* node, const std::string& sectionScope) const;
        std::shared_ptr<Value> visit(Ast::MapLiteral* node, const std::string& sectionScope) const;
        std::shared_ptr<Value> visit(Ast::FunctionCall* node, const std::string& sectionScope) const;

        // --- Data Stores ---

        // Stores the AST nodes for lazy evaluation
        std::map<std::string, SectionAst> m_section_asts;
        std::map<std::string, Ast::Expression*> m_define_asts;

        // Caches the computed values
        mutable std::map<std::string, SectionData> m_section_cache;
        mutable std::map<std::string, std::shared_ptr<Value>> m_define_cache;

        // Stores runtime errors
        mutable std::vector<YiniError> m_runtime_errors;

        // Tracks which keys are dynamic ("section.key")
        std::set<std::string> m_dynamic_keys;
    };
}

#endif // YINI_RUNTIME_H
