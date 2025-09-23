#ifndef YINI_RUNTIME_H
#define YINI_RUNTIME_H

#include "../Parser/Ast.h"
#include "../Common/Error.h"
#include "Value.h"
#include <map>
#include <string>
#include <memory>
#include <set>
#include <vector>

namespace Yini
{
    using SectionData = std::map<std::string, std::shared_ptr<Value>>;

    class YiniRuntime
    {
    public:
        YiniRuntime() = default;

        // Main entry points for loading
        bool loadFromFile(const std::string& filepath);
        void loadFromString(const std::string& content);

        // Public API to get values
        std::shared_ptr<Value> getValue(const std::string& sectionName, const std::string& key) const;

        // Public API to set values for Dyna
        bool setValue(const std::string& sectionName, const std::string& key, std::shared_ptr<Value> value);

        const std::vector<YiniError>& getErrors() const;

        // For debugging: dump the whole state
        std::string dump() const;

        // Serialization
        bool serialize(const std::string& filepath) const;
        bool deserialize(const std::string& filepath);

    private:
        // Visitor methods for each AST node type
        void evaluate(Ast::YiniDocument* doc);
        void loadAndMerge(const std::string& filepath, std::set<std::string>& processedFiles);
        std::shared_ptr<Value> visit(Ast::Node* node);
        std::shared_ptr<Value> visit(Ast::Expression* node);
        void visit(Ast::Section* node); // Sections don't return a value
        void visit(Ast::KeyValuePair* node);

        std::shared_ptr<Value> visit(Ast::IntegerLiteral* node);
        std::shared_ptr<Value> visit(Ast::FloatLiteral* node);
        std::shared_ptr<Value> visit(Ast::BooleanLiteral* node);
        std::shared_ptr<Value> visit(Ast::StringLiteral* node);
        std::shared_ptr<Value> visit(Ast::ColorLiteral* node);
        std::shared_ptr<Value> visit(Ast::InfixExpression* node);
        std::shared_ptr<Value> visit(Ast::MacroReference* node);
        std::shared_ptr<Value> visit(Ast::ArrayLiteral* node);
        std::shared_ptr<Value> visit(Ast::MapLiteral* node);
        std::shared_ptr<Value> visit(Ast::FunctionCall* node);

        // Helper to get the current section being processed
        SectionData& getCurrentSection();
        std::string m_currentSectionName;

        // --- Data Stores ---
        std::map<std::string, SectionData> m_sections;
        std::map<std::string, std::shared_ptr<Value>> m_defines;

        // Stores runtime errors
        std::vector<YiniError> m_runtime_errors;

        // Tracks which keys are dynamic ("section.key")
        std::set<std::string> m_dynamic_keys;
    };
}

#endif // YINI_RUNTIME_H
