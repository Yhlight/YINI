#include "Resolver.h"
#include <stdexcept>

namespace YINI
{
    void Resolver::resolve(AstNode& ast)
    {
        for (auto& section : ast.sections)
        {
            for (auto& kv : section.key_values)
            {
                if (auto* ref = std::get_if<CrossSectionRef>(&kv.value->value))
                {
                    YiniValue* foundValue = findValue(ast, ref->section, ref->key);
                    if (foundValue)
                    {
                        kv.value = foundValue->clone();
                    }
                    else
                    {
                        throw std::runtime_error("Could not resolve reference: " + ref->section + "." + ref->key);
                    }
                }
            }
        }
    }

    YiniValue* Resolver::findValue(AstNode& ast, const std::string& sectionName, const std::string& keyName)
    {
        for (auto& section : ast.sections)
        {
            if (section.name == sectionName)
            {
                for (auto& kv : section.key_values)
                {
                    if (kv.key == keyName)
                    {
                        return kv.value.get();
                    }
                }
            }
        }
        return nullptr;
    }
}