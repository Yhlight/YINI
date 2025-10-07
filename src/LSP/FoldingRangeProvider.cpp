#include "LSP/FoldingRangeProvider.h"
#include <sstream>

namespace yini::lsp
{

FoldingRangeProvider::FoldingRangeProvider()
{
}

json FoldingRangeProvider::makeFoldingRange(int startLine, int endLine, const std::string& kind)
{
    json range = {
        {"startLine", startLine},
        {"endLine", endLine}
    };
    
    if (!kind.empty())
    {
        range["kind"] = kind;
    }
    
    return range;
}

void FoldingRangeProvider::findSectionRanges(const std::string& content, json& ranges)
{
    std::istringstream stream(content);
    std::string line;
    int lineNum = 0;
    int sectionStart = -1;
    
    while (std::getline(stream, line))
    {
        // Check if line is a section header
        if (line.find('[') != std::string::npos && line.find(']') != std::string::npos)
        {
            // If we were in a section, close it
            if (sectionStart >= 0 && lineNum > sectionStart + 1)
            {
                ranges.push_back(makeFoldingRange(sectionStart, lineNum - 1, "region"));
            }
            
            // Start new section
            sectionStart = lineNum;
        }
        
        lineNum++;
    }
    
    // Close last section
    if (sectionStart >= 0 && lineNum > sectionStart + 1)
    {
        ranges.push_back(makeFoldingRange(sectionStart, lineNum - 1, "region"));
    }
}

void FoldingRangeProvider::findCommentRanges(const std::string& content, json& ranges)
{
    std::istringstream stream(content);
    std::string line;
    int lineNum = 0;
    int commentStart = -1;
    bool inBlockComment = false;
    
    while (std::getline(stream, line))
    {
        // Check for block comment start
        if (!inBlockComment && line.find("/*") != std::string::npos)
        {
            commentStart = lineNum;
            inBlockComment = true;
        }
        
        // Check for block comment end
        if (inBlockComment && line.find("*/") != std::string::npos)
        {
            if (lineNum > commentStart)
            {
                ranges.push_back(makeFoldingRange(commentStart, lineNum, "comment"));
            }
            inBlockComment = false;
        }
        
        lineNum++;
    }
}

json FoldingRangeProvider::getFoldingRanges(
    yini::Parser* /*parser*/,
    const std::string& content)
{
    json ranges = json::array();
    
    // Find section ranges
    findSectionRanges(content, ranges);
    
    // Find comment ranges
    findCommentRanges(content, ranges);
    
    return ranges;
}

} // namespace yini::lsp
