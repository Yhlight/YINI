#include "LSP/SymbolProvider.h"
#include <sstream>

namespace yini::lsp
{

SymbolProvider::SymbolProvider()
{
}

Position SymbolProvider::findSectionPosition(const std::string& content, const std::string& section)
{
    std::istringstream stream(content);
    std::string line;
    int line_num = 0;
    
    std::string target = "[" + section + "]";
    
    while (std::getline(stream, line))
    {
        size_t pos = line.find(target);
        if (pos != std::string::npos)
        {
            return {line_num, static_cast<int>(pos)};
        }
        line_num++;
    }
    
    return {0, 0};
}

Position SymbolProvider::findKeyPosition(const std::string& content, const std::string& section, const std::string& key)
{
    std::istringstream stream(content);
    std::string line;
    int line_num = 0;
    bool in_target_section = false;
    
    std::string section_header = "[" + section + "]";
    
    while (std::getline(stream, line))
    {
        // Check if we entered target section
        if (line.find(section_header) != std::string::npos)
        {
            in_target_section = true;
            line_num++;
            continue;
        }
        
        // Check if we entered another section
        if (in_target_section && line.find("[") != std::string::npos && line.find("]") != std::string::npos)
        {
            break;
        }
        
        // Look for key
        if (in_target_section)
        {
            size_t equals_pos = line.find('=');
            if (equals_pos != std::string::npos)
            {
                std::string found_key = line.substr(0, equals_pos);
                // Trim whitespace
                size_t start = found_key.find_first_not_of(" \t");
                size_t end = found_key.find_last_not_of(" \t");
                if (start != std::string::npos && end != std::string::npos)
                {
                    found_key = found_key.substr(start, end - start + 1);
                    if (found_key == key)
                    {
                        return {line_num, static_cast<int>(start)};
                    }
                }
            }
        }
        
        line_num++;
    }
    
    return {0, 0};
}

json SymbolProvider::makeSymbol(
    const std::string& name,
    int kind,
    Range range,
    Range selectionRange,
    const json& children)
{
    json symbol = {
        {"name", name},
        {"kind", kind},
        {"range", {
            {"start", {{"line", range.start.line}, {"character", range.start.character}}},
            {"end", {{"line", range.end.line}, {"character", range.end.character}}}
        }},
        {"selectionRange", {
            {"start", {{"line", selectionRange.start.line}, {"character", selectionRange.start.character}}},
            {"end", {{"line", selectionRange.end.line}, {"character", selectionRange.end.character}}}
        }}
    };
    
    if (!children.empty())
    {
        symbol["children"] = children;
    }
    
    return symbol;
}

json SymbolProvider::getDocumentSymbols(
    yini::Parser* parser,
    const std::string& content)
{
    if (!parser)
    {
        return json::array();
    }
    
    json symbols = json::array();
    
    // Add [#define] section symbols
    const auto& defines = parser->getDefines();
    if (!defines.empty())
    {
        json defineChildren = json::array();
        
        for (const auto& [name, value] : defines)
        {
            Position pos = findKeyPosition(content, "#define", name);
            Range range = {{pos.line, pos.character}, {pos.line, pos.character + static_cast<int>(name.length())}};
            
            defineChildren.push_back(makeSymbol(
                name,
                SYMBOL_VARIABLE,
                range,
                range
            ));
        }
        
        Position definePos = findSectionPosition(content, "#define");
        Range defineRange = {{definePos.line, 0}, {definePos.line + static_cast<int>(defines.size()), 0}};
        
        symbols.push_back(makeSymbol(
            "[#define]",
            SYMBOL_NAMESPACE,
            defineRange,
            {{definePos.line, definePos.character}, {definePos.line, definePos.character + 9}},
            defineChildren
        ));
    }
    
    // Add regular sections
    const auto& sections = parser->getSections();
    for (const auto& [section_name, section] : sections)
    {
        json sectionChildren = json::array();
        
        // Add keys in section
        for (const auto& [key, value] : section.entries)
        {
            (void)value; // Unused
            Position pos = findKeyPosition(content, section_name, key);
            Range range = {{pos.line, pos.character}, {pos.line, pos.character + static_cast<int>(key.length())}};
            
            sectionChildren.push_back(makeSymbol(
                key,
                SYMBOL_PROPERTY,
                range,
                range
            ));
        }
        
        Position sectionPos = findSectionPosition(content, section_name);
        Range sectionRange = {{sectionPos.line, 0}, {sectionPos.line + static_cast<int>(section.entries.size()), 0}};
        
        symbols.push_back(makeSymbol(
            "[" + section_name + "]",
            SYMBOL_CLASS,
            sectionRange,
            {{sectionPos.line, sectionPos.character}, {sectionPos.line, sectionPos.character + static_cast<int>(section_name.length()) + 2}},
            sectionChildren
        ));
    }
    
    return symbols;
}

} // namespace yini::lsp
