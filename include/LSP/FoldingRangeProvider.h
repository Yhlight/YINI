#ifndef YINI_FOLDING_RANGE_PROVIDER_H
#define YINI_FOLDING_RANGE_PROVIDER_H

#include "Parser.h"
#include <nlohmann/json.hpp>
#include <string>

namespace yini::lsp
{

using json = nlohmann::json;

class FoldingRangeProvider
{
public:
    FoldingRangeProvider();
    
    // Get folding ranges for document
    json getFoldingRanges(
        yini::Parser* parser,
        const std::string& content
    );
    
private:
    // Create folding range
    json makeFoldingRange(int startLine, int endLine, const std::string& kind = "");
    
    // Find section ranges
    void findSectionRanges(const std::string& content, json& ranges);
    
    // Find comment ranges
    void findCommentRanges(const std::string& content, json& ranges);
};

} // namespace yini::lsp

#endif // YINI_FOLDING_RANGE_PROVIDER_H
