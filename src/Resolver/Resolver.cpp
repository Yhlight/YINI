#include "Resolver.h"
#include <stdexcept>
#include <map>

namespace Yini
{

Resolver::Resolver(std::vector<std::unique_ptr<SectionNode>>& ast) : ast(ast) {}

void Resolver::resolve()
{
    // First, collect all macro definitions.
    for (const auto& section : ast)
    {
        if (section->special_type == SpecialSectionType::Define) {
            for (const auto& pair : section->pairs) {
                macroMap[pair->key.lexeme] = pair->value.get();
            }
        }
    }

    // Now, build a map of all non-special sections for quick lookup.
    for (const auto& section : ast)
    {
        if (section->special_type == SpecialSectionType::None) {
            if (sectionMap.count(section->name.lexeme))
            {
                throw std::runtime_error("Duplicate section name: " + section->name.lexeme);
            }
            sectionMap[section->name.lexeme] = section.get();
        }
    }

    // Resolve references and inheritance for each non-special section.
    for (const auto& section : ast)
    {
        if (section->special_type == SpecialSectionType::None) {
            resolveSection(*section);
        }
    }
}

std::unique_ptr<Value> Resolver::resolveValue(Value* value) {
    if (value->getType() == ValueType::Reference) {
        auto* refValue = dynamic_cast<ReferenceValue*>(value);
        auto it = macroMap.find(refValue->token.lexeme);
        if (it == macroMap.end()) {
            throw std::runtime_error("Undefined macro reference: " + refValue->token.lexeme);
        }
        // Recursively resolve the macro's value in case it contains other references.
        return resolveValue(it->second);
    }
    // For other types, just clone them.
    return value->clone();
}


void Resolver::resolveSection(SectionNode& section)
{
    // First, resolve all references within the section's own key-value pairs
    for (auto& pair : section.pairs) {
        pair->value = resolveValue(pair->value.get());
    }

    if (section.parents.empty())
    {
        return; // Nothing more to resolve
    }

    std::map<std::string, std::unique_ptr<Value>> resolvedPairs;

    for (const auto& parentNameToken : section.parents)
    {
        auto it = sectionMap.find(parentNameToken.lexeme);
        if (it == sectionMap.end())
        {
            throw std::runtime_error("Parent section not found: " + parentNameToken.lexeme);
        }

        SectionNode* parent = it->second;
        resolveSection(*parent);

        for (const auto& parentPair : parent->pairs)
        {
            resolvedPairs[parentPair->key.lexeme] = parentPair->value->clone();
        }
    }

    for (const auto& childPair : section.pairs)
    {
        resolvedPairs[childPair->key.lexeme] = childPair->value->clone();
    }

    section.pairs.clear();
    for (auto& pair : resolvedPairs)
    {
        Token keyToken;
        keyToken.lexeme = pair.first;
        section.pairs.push_back(std::make_unique<KeyValuePairNode>(keyToken, std::move(pair.second)));
    }

    section.parents.clear();
}

} // namespace Yini