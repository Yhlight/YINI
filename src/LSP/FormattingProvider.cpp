#include "LSP/FormattingProvider.h"
#include <sstream>
#include <cctype>

namespace yini::lsp
{

FormattingProvider::FormattingProvider()
{
}

std::string FormattingProvider::trimTrailing(const std::string& line)
{
    size_t end = line.find_last_not_of(" \t\r\n");
    if (end == std::string::npos)
    {
        return "";
    }
    return line.substr(0, end + 1);
}

bool FormattingProvider::isSectionHeader(const std::string& line)
{
    std::string trimmed = trimTrailing(line);
    size_t start = trimmed.find_first_not_of(" \t");
    if (start == std::string::npos)
    {
        return false;
    }
    
    return trimmed[start] == '[' && trimmed.find(']') != std::string::npos;
}

bool FormattingProvider::isKeyValuePair(const std::string& line)
{
    return line.find('=') != std::string::npos;
}

int FormattingProvider::getIndentLevel(const std::string& line)
{
    if (isSectionHeader(line))
    {
        return 0;
    }
    if (isKeyValuePair(line))
    {
        return 1;
    }
    return 0;
}

std::string FormattingProvider::formatLine(const std::string& line, const FormattingOptions& options)
{
    // Trim the line
    std::string trimmed = trimTrailing(line);
    size_t start = trimmed.find_first_not_of(" \t");
    
    if (start == std::string::npos)
    {
        // Empty line
        return "";
    }
    
    trimmed = trimmed.substr(start);
    
    // Get indent level
    int indent_level = getIndentLevel(trimmed);
    
    // Create indentation
    std::string indent;
    if (options.insertSpaces)
    {
        indent = std::string(indent_level * options.tabSize, ' ');
    }
    else
    {
        indent = std::string(indent_level, '\t');
    }
    
    // Format key-value pairs
    if (isKeyValuePair(trimmed))
    {
        size_t equals_pos = trimmed.find('=');
        if (equals_pos != std::string::npos)
        {
            // Get key and value
            std::string key = trimmed.substr(0, equals_pos);
            std::string value = trimmed.substr(equals_pos + 1);
            
            // Trim key and value
            size_t key_end = key.find_last_not_of(" \t");
            if (key_end != std::string::npos)
            {
                key = key.substr(0, key_end + 1);
            }
            
            size_t value_start = value.find_first_not_of(" \t");
            if (value_start != std::string::npos)
            {
                value = value.substr(value_start);
            }
            
            // Format as "key = value"
            return indent + key + " = " + value;
        }
    }
    
    // Return with proper indentation
    return indent + trimmed;
}

json FormattingProvider::makeTextEdit(int line, int startChar, int endChar, const std::string& newText)
{
    return {
        {"range", {
            {"start", {{"line", line}, {"character", startChar}}},
            {"end", {{"line", line}, {"character", endChar}}}
        }},
        {"newText", newText}
    };
}

json FormattingProvider::formatDocument(
    const std::string& content,
    const FormattingOptions& options)
{
    json edits = json::array();
    std::istringstream stream(content);
    std::string line;
    int line_num = 0;
    
    while (std::getline(stream, line))
    {
        std::string formatted = formatLine(line, options);
        
        // Only create edit if line changed
        if (formatted != line)
        {
            if (options.trimTrailingWhitespace)
            {
                formatted = trimTrailing(formatted);
            }
            
            edits.push_back(makeTextEdit(
                line_num,
                0,
                static_cast<int>(line.length()),
                formatted
            ));
        }
        
        line_num++;
    }
    
    // Add final newline if requested
    if (options.insertFinalNewline && !content.empty() && content.back() != '\n')
    {
        edits.push_back(makeTextEdit(
            line_num - 1,
            static_cast<int>(content.length()),
            static_cast<int>(content.length()),
            "\n"
        ));
    }
    
    return edits;
}

json FormattingProvider::formatRange(
    const std::string& content,
    Range range,
    const FormattingOptions& options)
{
    json edits = json::array();
    std::istringstream stream(content);
    std::string line;
    int line_num = 0;
    
    while (std::getline(stream, line))
    {
        // Only format lines in range
        if (line_num >= range.start.line && line_num <= range.end.line)
        {
            std::string formatted = formatLine(line, options);
            
            if (formatted != line)
            {
                if (options.trimTrailingWhitespace)
                {
                    formatted = trimTrailing(formatted);
                }
                
                edits.push_back(makeTextEdit(
                    line_num,
                    0,
                    static_cast<int>(line.length()),
                    formatted
                ));
            }
        }
        
        line_num++;
    }
    
    return edits;
}

} // namespace yini::lsp
