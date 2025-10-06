#include "LSP/RenameProvider.h"
#include <sstream>
#include <cctype>

namespace yini::lsp
{

RenameProvider::RenameProvider()
{
}

std::string RenameProvider::getLineAtPosition(const std::string& content, int line)
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

std::string RenameProvider::getWordAtPosition(const std::string& content, Position pos)
{
    std::string line = getLineAtPosition(content, pos.line);
    if (line.empty() || pos.character >= static_cast<int>(line.length()))
    {
        return "";
    }
    
    int start = pos.character;
    int end = pos.character;
    
    while (start > 0 && (std::isalnum(line[start - 1]) || line[start - 1] == '_'))
    {
        start--;
    }
    
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

bool RenameProvider::isMacroReference(const std::string& line, int character)
{
    for (int i = character - 1; i >= 0; i--)
    {
        if (line[i] == '@')
        {
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

bool RenameProvider::isValidIdentifier(const std::string& name)
{
    if (name.empty())
    {
        return false;
    }
    
    // First character must be letter or underscore
    if (!std::isalpha(name[0]) && name[0] != '_')
    {
        return false;
    }
    
    // Rest must be alphanumeric or underscore
    for (size_t i = 1; i < name.length(); i++)
    {
        if (!std::isalnum(name[i]) && name[i] != '_')
        {
            return false;
        }
    }
    
    return true;
}

json RenameProvider::makeTextEdit(int line, int startChar, int endChar, const std::string& newText)
{
    return {
        {"range", {
            {"start", {{"line", line}, {"character", startChar}}},
            {"end", {{"line", line}, {"character", endChar}}}
        }},
        {"newText", newText}
    };
}

json RenameProvider::createMacroRenameEdits(
    const std::string& content,
    const std::string& oldName,
    const std::string& newName)
{
    json edits = json::array();
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
        }
        else if (in_define_section && line.find("[") != std::string::npos && line.find("]") != std::string::npos)
        {
            in_define_section = false;
        }
        
        // Rename declaration in [#define]
        if (in_define_section)
        {
            size_t equals_pos = line.find('=');
            if (equals_pos != std::string::npos)
            {
                std::string key = line.substr(0, equals_pos);
                size_t start = key.find_first_not_of(" \t");
                size_t end = key.find_last_not_of(" \t");
                if (start != std::string::npos && end != std::string::npos)
                {
                    key = key.substr(start, end - start + 1);
                    if (key == oldName)
                    {
                        edits.push_back(makeTextEdit(
                            line_num,
                            static_cast<int>(start),
                            static_cast<int>(end + 1),
                            newName
                        ));
                    }
                }
            }
        }
        
        // Rename all @oldName references
        std::string pattern = "@" + oldName;
        size_t pos = 0;
        while ((pos = line.find(pattern, pos)) != std::string::npos)
        {
            // Check if it's a whole word
            if (pos + pattern.length() < line.length())
            {
                char next = line[pos + pattern.length()];
                if (!std::isalnum(next) && next != '_')
                {
                    edits.push_back(makeTextEdit(
                        line_num,
                        static_cast<int>(pos + 1), // Skip @
                        static_cast<int>(pos + pattern.length()),
                        newName
                    ));
                }
            }
            else
            {
                edits.push_back(makeTextEdit(
                    line_num,
                    static_cast<int>(pos + 1), // Skip @
                    static_cast<int>(pos + pattern.length()),
                    newName
                ));
            }
            pos += pattern.length();
        }
        
        line_num++;
    }
    
    return edits;
}

json RenameProvider::prepareRename(
    yini::Parser* /*parser*/,
    const std::string& content,
    Position position)
{
    std::string line = getLineAtPosition(content, position.line);
    if (line.empty())
    {
        return nullptr;
    }
    
    std::string word = getWordAtPosition(content, position);
    if (word.empty())
    {
        return nullptr;
    }
    
    // Check if it's a valid symbol to rename (macro for now)
    if (isMacroReference(line, position.character))
    {
        // Return the range of the word
        int start = position.character;
        while (start > 0 && (std::isalnum(line[start - 1]) || line[start - 1] == '_'))
        {
            start--;
        }
        
        int end = position.character;
        while (end < static_cast<int>(line.length()) && (std::isalnum(line[end]) || line[end] == '_'))
        {
            end++;
        }
        
        return {
            {"range", {
                {"start", {{"line", position.line}, {"character", start}}},
                {"end", {{"line", position.line}, {"character", end}}}
            }},
            {"placeholder", word}
        };
    }
    
    return nullptr;
}

json RenameProvider::rename(
    yini::Parser* /*parser*/,
    const std::string& content,
    const std::string& uri,
    Position position,
    const std::string& newName)
{
    // Validate new name
    if (!isValidIdentifier(newName))
    {
        return nullptr;
    }
    
    std::string line = getLineAtPosition(content, position.line);
    if (line.empty())
    {
        return nullptr;
    }
    
    std::string oldName = getWordAtPosition(content, position);
    if (oldName.empty() || oldName == newName)
    {
        return nullptr;
    }
    
    // For now, only support macro renaming
    if (isMacroReference(line, position.character))
    {
        json edits = createMacroRenameEdits(content, oldName, newName);
        
        if (edits.empty())
        {
            return nullptr;
        }
        
        return {
            {"changes", {
                {uri, edits}
            }}
        };
    }
    
    return nullptr;
}

} // namespace yini::lsp
