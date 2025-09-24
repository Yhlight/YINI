#include "Resolver.h"
#include <iostream>

namespace YINI
{
    // NOTE: A production-ready clone function would be required for safe AST transformation.
    // It would need to handle every AST node type and perform a deep copy.
    // This is a complex task and is omitted for this stage of development.
    // The current resolver identifies where substitution should happen but does not perform it.
    /*
    static std::unique_ptr<AST::Expression> cloneExpression(AST::Expression* original)
    {
        if (!original) return nullptr;
        // ... implementation for each expression type ...
    }
    */

    Resolver::Resolver(AST::Program& program) : m_program(program) {}

    void Resolver::resolve()
    {
        // Step 1: Walk the AST to find all macro definitions.
        gatherMacros(&m_program);

        // Step 2: Walk the AST again to substitute any macro references.
        substituteMacros(&m_program);

        // Future steps would include:
        // - Resolving section inheritance.
        // - Processing file includes.
    }

    void Resolver::gatherMacros(AST::Node* node)
    {
        if (!node) return;

        // Check if the node is a [#define] section
        if (auto* section = dynamic_cast<AST::Section*>(node))
        {
            if (section->name->value == "#define")
            {
                for (const auto& stmt : section->statements)
                {
                    // In a [#define] section, statements should be key-value pairs.
                    if (auto* kvp = dynamic_cast<AST::KeyValuePair*>(stmt.get()))
                    {
                        m_macro_table[kvp->key->value] = kvp->value.get();
                    }
                }
                // We could remove the [#define] section from the AST after processing it.
            }
        }

        // Recurse through children to find all [#define] sections.
        if (auto* program = dynamic_cast<AST::Program*>(node))
        {
            for (auto& stmt : program->statements)
            {
                gatherMacros(stmt.get());
            }
        }
        else if (auto* section = dynamic_cast<AST::Section*>(node))
        {
            for (auto& stmt : section->statements)
            {
                gatherMacros(stmt.get());
            }
        }
    }

    void Resolver::substituteMacros(AST::Node* node)
    {
        if (!node) return;

        // To properly substitute, we would need a visitor pattern that can
        // return a new unique_ptr to replace the visited node. This is a placeholder
        // that demonstrates the identification of a substitution site.
        if (auto* kvp = dynamic_cast<AST::KeyValuePair*>(node))
        {
            // We need to traverse the expression tree on the right-hand side.
            // This placeholder only checks one level deep for an InfixExpression.
            // A full implementation would be a recursive walk of the expression.
            if (auto* infix_expr = dynamic_cast<AST::InfixExpression*>(kvp->value.get()))
            {
                if (auto* macro_ref = dynamic_cast<AST::MacroReference*>(infix_expr->left.get()))
                {
                    std::string macro_name = macro_ref->name->value;
                    if (m_macro_table.count(macro_name))
                    {
                        // In a full implementation, we would replace this node.
                        // Example: infix_expr->left = cloneExpression(m_macro_table.at(macro_name));
                        std::cout << "INFO: Found macro reference @" << macro_name << " to be substituted." << std::endl;
                    }
                }
            }
        }

        // Recurse through the AST to find all substitution sites.
        if (auto* program = dynamic_cast<AST::Program*>(node))
        {
            for (auto& stmt : program->statements)
            {
                substituteMacros(stmt.get());
            }
        }
        else if (auto* section = dynamic_cast<AST::Section*>(node))
        {
            for (auto& stmt : section->statements)
            {
                substituteMacros(stmt.get());
            }
        }
    }
}
