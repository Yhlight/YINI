#pragma once

#include "Parser/Ast.h"
#include <string>
#include <vector>
#include <memory>

namespace Yini
{

class Loader
{
public:
    std::vector<std::unique_ptr<SectionNode>> load(const std::string& filepath);

private:
    std::vector<std::unique_ptr<SectionNode>> parseFile(const std::string& filepath, std::vector<std::string>& loaded_files);
    void mergeAst(std::vector<std::unique_ptr<SectionNode>>& base, std::vector<std::unique_ptr<SectionNode>>& to_merge);
    void mergeAst(std::vector<std::unique_ptr<SectionNode>>& base, std::unique_ptr<SectionNode>& section_to_merge);
};

} // namespace Yini