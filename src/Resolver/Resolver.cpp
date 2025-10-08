#include "Resolver.h"
#include <stdexcept>

namespace Yini
{

Resolver::Resolver(std::vector<std::unique_ptr<SectionNode>>& ast) : ast(ast) {}

void Resolver::resolve()
{
    // First, build a map of all sections for quick lookup.
    for (const auto& section : ast)
    {
        if (sectionMap.find(section->name.lexeme) != sectionMap.end())
        {
            // In the future, we might allow sections to be redefined and merged.
            // For now, it's an error.
            throw std::runtime_error("Duplicate section name: " + section->name.lexeme);
        }
        sectionMap[section->name.lexeme] = section.get();
    }

    // Now, resolve inheritance for each section.
    for (const auto& section : ast)
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

    // A map to keep track of keys in the current section to handle overrides.
    std::map<std::string, bool> childKeys;
    for(const auto& pair : section.pairs) {
        childKeys[pair->key.lexeme] = true;
    }

    // Inherit from parents in reverse order so that later parents override earlier ones.
    // The YINI.md spec says "later inherited configuration blocks override previously inherited or existing key-value pairs".
    // So we iterate normally, and the logic below will handle the override correctly.
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

        // Inherit pairs from the parent.
        for (const auto& parentPair : parent->pairs)
        {
            // If the key already exists in the child, the child's value takes precedence.
            // The spec says later parents override earlier ones, and child keys override all parents.
            // Let's create a new list of pairs for the child.

            // This is getting complicated. Let's simplify.
            // We can create a new vector of pairs and replace the child's pairs.
            // Let's rethink the override logic.

            // The order is: Child's own keys > last parent's keys > ... > first parent's keys.
            // A simple way is to build a map of the final key-value pairs.
        }
    }

    // Let's try a simpler approach for now.
    // Create a new list of pairs and add the parents' pairs first, then the child's.
    // This will let the child's keys override the parents'.
    std::vector<std::unique_ptr<KeyValuePairNode>> newPairs;
    std::map<std::string, bool> keysInNewList;

    // Add pairs from parents first.
    for (const auto& parentNameToken : section.parents) {
        SectionNode* parent = sectionMap[parentNameToken.lexeme];
        for(const auto& parentPair : parent->pairs) {
            if(keysInNewList.find(parentPair->key.lexeme) == keysInNewList.end()) {
                 keysInNewList[parentPair->key.lexeme] = true;
                 // This is a deep copy, which is what we want.
                 auto value = std::make_unique<ValueNode>(parentPair->value->token);
                 newPairs.push_back(std::make_unique<KeyValuePairNode>(parentPair->key, std::move(value)));
            } else {
                // Find and update the existing key
                for(auto& pair : newPairs) {
                    if(pair->key.lexeme == parentPair->key.lexeme) {
                        pair->value = std::make_unique<ValueNode>(parentPair->value->token);
                        break;
                    }
                }
            }
        }
    }

    // Add the child's own pairs, overriding any from parents.
    for(const auto& childPair : section.pairs) {
        if(keysInNewList.find(childPair->key.lexeme) == keysInNewList.end()) {
            keysInNewList[childPair->key.lexeme] = true;
            auto value = std::make_unique<ValueNode>(childPair->value->token);
            newPairs.push_back(std::make_unique<KeyValuePairNode>(childPair->key, std::move(value)));
        } else {
             for(auto& pair : newPairs) {
                    if(pair->key.lexeme == childPair->key.lexeme) {
                        pair->value = std::make_unique<ValueNode>(childPair->value->token);
                        break;
                    }
                }
        }
    }


    section.pairs = std::move(newPairs);
    // Clear parents so we don't resolve them again.
    section.parents.clear();

}

} // namespace Yini