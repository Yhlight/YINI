#include "LSP/DefinitionProvider.h"
#include <sstream>
#include <cctype>

namespace yini::lsp
{

DefinitionProvider::DefinitionProvider()
{
}

std::string DefinitionProvider::getLineAtPosition(const std::string& content, int line)
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

std::string DefinitionProvider::getWordAtPosition(const std::string& content, Position pos)
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

bool DefinitionProvider::isMacroReference(const std::string& line, int character)
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

bool DefinitionProvider::isCrossSectionReference(const std::string& line, int character)
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

json DefinitionProvider::makeLocation(const std::string& uri, int line, int character)
{
    return {
        {"uri", uri},
        {"range", {
            {"start", {{"line", line}, {"character", character}}},
            {"end", {{"line", line}, {"character", character}}}
        }}
    };
}

json DefinitionProvider::findMacroDefinition(const std::string& content, const std::string& uri, const std::string& name)
{
    std::istringstream stream(content);
    std::string line;
    int line_num = 0;
    bool in_define_section = false;
    
    while (std::getline(stream, line))
    {
        // Check for [#define] section
        if (line.find("[#define]") != std::string::npos)
        {
            in_define_section = true;
            line_num++;
            continue;
        }
        
        // Check for next section (exits [#define])
        if (in_define_section && line.find("[") != std::string::npos && line.find("]") != std::string::npos)
        {
            in_define_section = false;
        }
        
        // Look for macro definition
        if (in_define_section)
        {
            size_t equals_pos = line.find('=');
            if (equals_pos != std::string::npos)
            {
                std::string key = line.substr(0, equals_pos);
                // Trim whitespace
                size_t start = key.find_first_not_of(" \t");
                size_t end = key.find_last_not_of(" \t");
                if (start != std::string::npos && end != std::string::npos)
                {
                    key = key.substr(start, end - start + 1);
                    if (key == name)
                    {
                        return makeLocation(uri, line_num, static_cast<int>(start));
                    }
                }
            }
        }
        
        line_num++;
    }
    
    return nullptr;
}

json DefinitionProvider::findSectionDefinition(const std::string& content, const std::string& uri, const std::string& section)
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
            // Make sure it's a section header, not part of a comment or string
            return makeLocation(uri, line_num, static_cast<int>(pos));
        }
        line_num++;
    }
    
    return nullptr;
}

json DefinitionProvider::findKeyDefinition(const std::string& content, const std::string& uri, const std::string& section, const std::string& key)
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
            in_target_section = false;
        }
        
        // Look for key definition
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
                        return makeLocation(uri, line_num, static_cast<int>(start));
                    }
                }
            }
        }
        
        line_num++;
    }
    
    return nullptr;
}

json DefinitionProvider::getDefinition(
    yini::Parser* /*parser*/,
    const std::string& content,
    const std::string& uri,
    Position position)
{
    std::string line = getLineAtPosition(content, position.line);
    if (line.empty())
    {
        return nullptr;
    }
    
    // Check if clicking on macro reference (@name)
    if (isMacroReference(line, position.character))
    {
        std::string word = getWordAtPosition(content, position);
        if (!word.empty())
        {
            return findMacroDefinition(content, uri, word);
        }
    }
    
    // Check if clicking on cross-section reference (@{Section.key})
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
                    return findKeyDefinition(content, uri, section, key);
                }
                else
                {
                    // Just section name
                    return findSectionDefinition(content, uri, ref);
                }
            }
        }
    }
    
    return nullptr;
}

} // namespace yini::lsp
