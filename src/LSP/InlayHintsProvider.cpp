#include "LSP/InlayHintsProvider.h"
#include <sstream>

namespace yini::lsp
{

InlayHintsProvider::InlayHintsProvider()
{
}

json InlayHintsProvider::makeInlayHint(Position position, const std::string& label, const std::string& kind)
{
    json hint = {
        {"position", {{"line", position.line}, {"character", position.character}}},
        {"label", label}
    };
    
    if (!kind.empty())
    {
        hint["kind"] = kind == "type" ? 1 : 2; // 1=Type, 2=Parameter
    }
    
    return hint;
}

json InlayHintsProvider::getMacroHint(const std::string& macroName, yini::Parser* parser, Position position)
{
    if (!parser)
    {
        return nullptr;
    }
    
    const auto& defines = parser->getDefines();
    auto it = defines.find(macroName);
    
    if (it != defines.end())
    {
        std::string value = it->second->toString();
        return makeInlayHint(position, ": " + value, "type");
    }
    
    return nullptr;
}

json InlayHintsProvider::getTypeHint(const std::string& /*value*/, Position position)
{
    // Could show type information for values
    return makeInlayHint(position, ": int", "type");
}

json InlayHintsProvider::getInlayHints(
    yini::Parser* parser,
    const std::string& content,
    Range range)
{
    json hints = json::array();
    
    if (!parser)
    {
        return hints;
    }
    
    const auto& defines = parser->getDefines();
    
    std::istringstream stream(content);
    std::string line;
    int lineNum = 0;
    
    while (std::getline(stream, line))
    {
        if (lineNum < range.start.line || lineNum > range.end.line)
        {
            lineNum++;
            continue;
        }
        
        // Find macro references and show their values
        for (const auto& [macroName, value] : defines)
        {
            (void)value;
            std::string pattern = "@" + macroName;
            size_t pos = 0;
            
            while ((pos = line.find(pattern, pos)) != std::string::npos)
            {
                Position hintPos{lineNum, static_cast<int>(pos + pattern.length())};
                auto hint = getMacroHint(macroName, parser, hintPos);
                if (!hint.is_null())
                {
                    hints.push_back(hint);
                }
                pos += pattern.length();
            }
        }
        
        lineNum++;
    }
    
    return hints;
}

} // namespace yini::lsp
