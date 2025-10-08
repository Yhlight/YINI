#pragma once

#include "Parser/Ast.h"
#include <vector>
#include <memory>
#include <map>

namespace Yini
{
class Resolver
{
public:
    Resolver(std::vector<std::unique_ptr<SectionNode>>& ast);
    void resolve();

private:
    void resolveSection(SectionNode& section);
    void resolveMacros();
    std::unique_ptr<Value> resolveValue(Value* value);

    std::vector<std::unique_ptr<SectionNode>>& ast;
    std::map<std::string, SectionNode*> sectionMap;
    std::map<std::string, Value*> macroMap;
};
} // namespace Yini