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

// Helper to convert TokenType to string
std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::INTEGER: return "integer";
        case TokenType::FLOAT: return "float";
        case TokenType::BOOLEAN: return "boolean";
        case TokenType::STRING: return "string";
        case TokenType::IDENTIFIER: return "identifier";
        case TokenType::COLOR: return "color";
        case TokenType::COORD: return "coordinate";
        case TokenType::PATH: return "path";
        case TokenType::LIST: return "list";
        case TokenType::ARRAY: return "array";
        case TokenType::MAP: return "map";
        case TokenType::DYNA: return "dynamic";
        case TokenType::SECTION_START: return "section_start";
        default: return "token";
    }
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

std::optional<Token> HoverProvider::findTokenAtPosition(yini::Parser* parser, Position position)
{
    if (!parser) {
        return std::nullopt;
    }

    const auto& tokens = parser->getTokens();
    for (const auto& token : tokens) {
        // LSP position is 0-based, our parser is 1-based.
        size_t lsp_line = token.line - 1;
        size_t lsp_col_start = token.column - 1;
        size_t lsp_col_end = lsp_col_start + token.length;

        if (lsp_line == static_cast<size_t>(position.line) &&
            static_cast<size_t>(position.character) >= lsp_col_start &&
            static_cast<size_t>(position.character) < lsp_col_end)
        {
            return token;
        }
    }

    return std::nullopt;
}

json HoverProvider::getHover(
    yini::Parser* parser,
    const std::string& /* content */,
    Position position)
{
    auto token_opt = findTokenAtPosition(parser, position);
    if (!token_opt)
    {
        return nullptr;
    }

    Token token = token_opt.value();
    
    // Create a hover content based on the token type
    std::ostringstream content;
    std::string type_str = tokenTypeToString(token.type);
    
    // Don't show hover for uninteresting tokens like operators or brackets
    switch(token.type) {
        case TokenType::INTEGER:
        case TokenType::FLOAT:
        case TokenType::BOOLEAN:
        case TokenType::STRING:
        case TokenType::IDENTIFIER:
        case TokenType::COLOR:
        case TokenType::COORD:
        case TokenType::PATH:
        case TokenType::LIST:
        case TokenType::ARRAY:
        case TokenType::MAP:
        case TokenType::DYNA:
            content << "**Type**: `" << type_str << "`\n\n";
            content << "**Value**: `" << token.toString() << "`";
            break;
        default:
            return nullptr; // No hover for other token types
    }

    return makeHoverContent(content.str());
}

} // namespace yini::lsp
