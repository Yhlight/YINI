#include "LSP/HoverProvider.h"
#include <sstream>
#include <cctype>

namespace yini::lsp
{

HoverProvider::HoverProvider()
{
}

std::string HoverProvider::getLineAtPosition(const std::string& content, int line)
{
    std::istringstream stream(content);
    std::string current_line;
    int current = 0;
    
    while (std::getline(stream, current_line))
    {
        if (current == line)
        {
            return current_line;
        }
        current++;
    }
    
    return "";
}

std::string HoverProvider::getWordAtPosition(const std::string& content, Position pos)
{
    std::string line = getLineAtPosition(content, pos.line);
    if (line.empty() || pos.character >= static_cast<int>(line.length()))
    {
        return "";
    }
    
    // Find word boundaries
    int start = pos.character;
    int end = pos.character;
    
    // Expand left
    while (start > 0 && (std::isalnum(line[start - 1]) || line[start - 1] == '_'))
    {
        start--;
    }
    
    // Expand right
    while (end < static_cast<int>(line.length()) && (std::isalnum(line[end]) || line[end] == '_'))
    {
        end++;
    }
    
    if (start < end)
    {
        return line.substr(start, end - start);
    }
    
    return "";
}

bool HoverProvider::isMacroReference(const std::string& line, int character)
{
    // Check if there's @ before the position
    for (int i = character - 1; i >= 0; i--)
    {
        if (line[i] == '@')
        {
            // Make sure it's not @{
            if (i + 1 < static_cast<int>(line.length()) && line[i + 1] == '{')
            {
                return false;
            }
            return true;
        }
        if (!std::isalnum(line[i]) && line[i] != '_')
        {
            break;
        }
    }
    return false;
}

bool HoverProvider::isCrossSectionReference(const std::string& line, int character)
{
    // Check if we're inside @{...}
    int brace_count = 0;
    for (int i = 0; i < character && i < static_cast<int>(line.length()); i++)
    {
        if (i + 1 < static_cast<int>(line.length()) && line[i] == '@' && line[i + 1] == '{')
        {
            brace_count++;
            i++; // Skip {
        }
        else if (line[i] == '}')
        {
            brace_count--;
        }
    }
    return brace_count > 0;
}

std::string HoverProvider::getValueTypeString(std::shared_ptr<yini::Value> value)
{
    if (!value)
    {
        return "unknown";
    }
    
    switch (value->getType())
    {
        case yini::ValueType::INTEGER:
            return "integer";
        case yini::ValueType::FLOAT:
            return "float";
        case yini::ValueType::BOOLEAN:
            return "boolean";
        case yini::ValueType::STRING:
            return "string";
        case yini::ValueType::ARRAY:
            return "array";
        case yini::ValueType::MAP:
            return "map";
        case yini::ValueType::COLOR:
            return "color";
        case yini::ValueType::COORD:
            return "coordinate";
        case yini::ValueType::LIST:
            return "list";
        case yini::ValueType::PATH:
            return "path";
        case yini::ValueType::DYNAMIC:
            return "dynamic";
        case yini::ValueType::REFERENCE:
            return "reference";
        case yini::ValueType::ENV_VAR:
            return "environment variable";
        default:
            return "unknown";
    }
}

json HoverProvider::getMacroHover(yini::Parser* parser, const std::string& name)
{
    if (!parser)
    {
        return nullptr;
    }
    
    const auto& defines = parser->getDefines();
    auto it = defines.find(name);
    
    if (it == defines.end())
    {
        return nullptr;
    }
    
    auto value = it->second;
    std::string type = getValueTypeString(value);
    std::string valueStr = value->toString();
    
    std::ostringstream content;
    content << "**Macro**: `@" << name << "`\n\n";
    content << "**Type**: `" << type << "`\n\n";
    content << "**Value**: `" << valueStr << "`\n\n";
    content << "Defined in `[#define]` section";
    
    return makeHoverContent(content.str());
}

json HoverProvider::getSectionKeyHover(yini::Parser* parser, const std::string& section, const std::string& key)
{
    if (!parser)
    {
        return nullptr;
    }
    
    const auto& sections = parser->getSections();
    auto it = sections.find(section);
    
    if (it == sections.end())
    {
        return nullptr;
    }
    
    const auto& entries = it->second.entries;
    auto key_it = entries.find(key);
    
    if (key_it == entries.end())
    {
        return nullptr;
    }
    
    auto value = key_it->second;
    std::string type = getValueTypeString(value);
    std::string valueStr = value->toString();
    
    std::ostringstream content;
    content << "**Section**: `[" << section << "]`\n\n";
    content << "**Key**: `" << key << "`\n\n";
    content << "**Type**: `" << type << "`\n\n";
    content << "**Value**: `" << valueStr << "`";
    
    return makeHoverContent(content.str());
}

json HoverProvider::makeHoverContent(const std::string& content, const std::string& /*language*/)
{
    return {
        {"contents", {
            {"kind", "markdown"},
            {"value", content}
        }}
    };
}

json HoverProvider::getHover(
    yini::Parser* parser,
    const std::string& content,
    Position position)
{
    std::string line = getLineAtPosition(content, position.line);
    if (line.empty())
    {
        return nullptr;
    }
    
    // Check if hovering over macro reference (@name)
    if (isMacroReference(line, position.character))
    {
        std::string word = getWordAtPosition(content, position);
        if (!word.empty())
        {
            return getMacroHover(parser, word);
        }
    }
    
    // Check if hovering over cross-section reference (@{Section.key})
    if (isCrossSectionReference(line, position.character))
    {
        // Extract section and key from @{Section.key}
        size_t at_brace = line.rfind("@{", position.character);
        if (at_brace != std::string::npos)
        {
            size_t close_brace = line.find("}", at_brace);
            if (close_brace != std::string::npos)
            {
                std::string ref = line.substr(at_brace + 2, close_brace - at_brace - 2);
                size_t dot_pos = ref.find('.');
                
                if (dot_pos != std::string::npos)
                {
                    std::string section = ref.substr(0, dot_pos);
                    std::string key = ref.substr(dot_pos + 1);
                    return getSectionKeyHover(parser, section, key);
                }
            }
        }
    }
    
    // Check if hovering over a key in current section
    std::string word = getWordAtPosition(content, position);
    if (!word.empty() && parser)
    {
        // Try to find in all sections (simple search)
        const auto& sections = parser->getSections();
        for (const auto& [section_name, section] : sections)
        {
            const auto& entries = section.entries;
            if (entries.find(word) != entries.end())
            {
                return getSectionKeyHover(parser, section_name, word);
            }
        }
        
        // Try to find as macro
        const auto& defines = parser->getDefines();
        if (defines.find(word) != defines.end())
        {
            return getMacroHover(parser, word);
        }
    }
    
    return nullptr;
}

} // namespace yini::lsp
