#ifndef YINI_RUNTIME_H
#define YINI_RUNTIME_H

#include "../Parser/Ast.h"
#include "Value.h"
#include <map>
#include <string>
#include <memory>

namespace Yini
{
    using SectionData = std::map<std::string, std::shared_ptr<Value>>;

    class YiniRuntime
    {
    public:
        YiniRuntime() = default;

        // The main entry point to evaluate the AST
        void evaluate(Ast::YiniDocument* doc);

        // Public API to get values
        std::shared_ptr<Value> getValue(const std::string& sectionName, const std::string& key) const;

        // For debugging: dump the whole state
        std::string dump() const;

        // Serialization
        bool serialize(const std::string& filepath) const;
        bool deserialize(const std::string& filepath);

    private:
        // Visitor methods for each AST node type
        std::shared_ptr<Value> visit(Ast::Node* node);
        std::shared_ptr<Value> visit(Ast::Expression* node);
        std::shared_ptr<Value> visit(Ast::KeyValuePair* node);
        std::shared_ptr<Value> visit(Ast::QuickRegister* node);
        std::shared_ptr<Value> visit(Ast::Section* node);
        std::shared_ptr<Value> visit(Ast::IntegerLiteral* node);
        std::shared_ptr<Value> visit(Ast::FloatLiteral* node);
        std::shared_ptr<Value> visit(Ast::BooleanLiteral* node);
        std::shared_ptr<Value> visit(Ast::StringLiteral* node);
        std::shared_ptr<Value> visit(Ast::InfixExpression* node);
        std::shared_ptr<Value> visit(Ast::Identifier* node);

        // Helper to get the current section being processed
        SectionData& getCurrentSection();

        std::map<std::string, SectionData> m_sections;
        std::map<std::string, std::shared_ptr<Value>> m_defines;
        std::string m_currentSectionName;
    };
}

#endif // YINI_RUNTIME_H
