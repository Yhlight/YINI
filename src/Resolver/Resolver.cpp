#include "Resolver.h"
#include <stdexcept>
#include <map>

namespace Yini
{

Resolver::Resolver(std::vector<std::unique_ptr<SectionNode>>& ast) : ast(ast) {}

void Resolver::resolve()
{
    // First, build a map of all sections for quick lookup.
    for (const auto& section : ast)
    {
        if (sectionMap.count(section->name.lexeme))
        {
            throw std::runtime_error("Duplicate section name: " + section->name.lexeme);
        }
        sectionMap[section->name.lexeme] = section.get();
    }

    // Now, resolve inheritance for each section.
    // We iterate through a copy of the pointers because the resolver might modify the ast vector itself
    // if we decide to merge sections, etc. For now, it's just safer.
    std::vector<SectionNode*> sectionsToResolve;
    for (const auto& section : ast) {
        sectionsToResolve.push_back(section.get());
    }

    for (auto* section : sectionsToResolve)
    {
        resolveSection(*section);
    }
}

void Resolver::resolveSection(SectionNode& section)
{
    if (section.parents.empty())
    {
        return; // Nothing to resolve
    }

    // This map will store the final, resolved key-value pairs for the section.
    // Using a map automatically handles overriding: the last one inserted wins.
    std::map<std::string, std::unique_ptr<Value>> resolvedPairs;

    // Inherit from parents. The spec says "later inherited...override previously inherited".
    // So we iterate through parents in order, and later parents will override earlier ones in the map.
    for (const auto& parentNameToken : section.parents)
    {
        auto it = sectionMap.find(parentNameToken.lexeme);
        if (it == sectionMap.end())
        {
            throw std::runtime_error("Parent section not found: " + parentNameToken.lexeme);
        }

        SectionNode* parent = it->second;

        // Recursively resolve the parent first to ensure its pairs are complete.
        resolveSection(*parent);

        // Inherit pairs from the parent. We need to deep-copy the values.
        for (const auto& parentPair : parent->pairs)
        {
            resolvedPairs[parentPair->key.lexeme] = parentPair->value->clone();
        }
    }

    // Finally, apply the child's own pairs, which will override any inherited pairs.
    for (const auto& childPair : section.pairs)
    {
        resolvedPairs[childPair->key.lexeme] = childPair->value->clone();
    }

    // Replace the section's old pairs with the new, resolved ones.
    section.pairs.clear();
    for (auto& pair : resolvedPairs)
    {
        Token keyToken;
        keyToken.lexeme = pair.first;
        section.pairs.push_back(std::make_unique<KeyValuePairNode>(keyToken, std::move(pair.second)));
    }

    // Clear parents so we don't resolve this section again unnecessarily.
    section.parents.clear();
}

} // namespace Yini