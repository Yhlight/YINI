#include "LSP/SemanticTokensProvider.h"
#include <sstream>
#include <algorithm>

namespace yini::lsp
{

SemanticTokensProvider::SemanticTokensProvider()
{
}

json SemanticTokensProvider::getLegend()
{
    return {
        {"tokenTypes", json::array({
            "namespace",   // 0
            "class",       // 1
            "enum",        // 2
            "interface",   // 3
            "struct",      // 4
            "typeParameter", // 5
            "parameter",   // 6
            "variable",    // 7
            "property",    // 8
            "enumMember",  // 9
            "decorator",   // 10
            "event",       // 11
            "function",    // 12
            "method",      // 13
            "macro",       // 14
            "label",       // 15
            "comment",     // 16
            "string",      // 17
            "keyword",     // 18
            "number",      // 19
            "regexp",      // 20
            "operator"     // 21
        })},
        {"tokenModifiers", json::array({
            "declaration",     // 0
            "definition",      // 1
            "readonly",        // 2
            "static",          // 3
            "deprecated",      // 4
            "abstract",        // 5
            "async",           // 6
            "modification",    // 7
            "documentation",   // 8
            "defaultLibrary"   // 9
        })}
    };
}

std::string SemanticTokensProvider::getLineAtPosition(const std::string& content, int line)
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

void SemanticTokensProvider::addToken(int line, int startChar, int length, SemanticTokenType type, int modifiers)
{
    tokens.push_back({line, startChar, length, type, modifiers});
}

void SemanticTokensProvider::findAndAddTokens(const std::string& line, int lineNum, const std::string& pattern, SemanticTokenType type)
{
    size_t pos = 0;
    while ((pos = line.find(pattern, pos)) != std::string::npos)
    {
        addToken(lineNum, static_cast<int>(pos), static_cast<int>(pattern.length()), type);
        pos += pattern.length();
    }
}

void SemanticTokensProvider::extractTokens(yini::Parser* parser, const std::string& content)
{
    tokens.clear();
    
    if (!parser)
    {
        return;
    }
    
    std::istringstream stream(content);
    std::string line;
    int lineNum = 0;
    bool inDefineSection = false;
    std::string currentSection;
    
    const auto& defines = parser->getDefines();
    
    while (std::getline(stream, line))
    {
        // Detect section headers
        if (line.find("[#define]") != std::string::npos)
        {
            size_t pos = line.find("[#define]");
            addToken(lineNum, static_cast<int>(pos), 9, SemanticTokenType::NAMESPACE, 1); // definition
            inDefineSection = true;
            currentSection = "#define";
        }
        else if (line.find("[#include]") != std::string::npos)
        {
            size_t pos = line.find("[#include]");
            addToken(lineNum, static_cast<int>(pos), 10, SemanticTokenType::NAMESPACE, 1);
            inDefineSection = false;
            currentSection = "#include";
        }
        else if (line.find("[#schema]") != std::string::npos)
        {
            size_t pos = line.find("[#schema]");
            addToken(lineNum, static_cast<int>(pos), 9, SemanticTokenType::NAMESPACE, 1);
            inDefineSection = false;
            currentSection = "#schema";
        }
        else if (line.find('[') != std::string::npos && line.find(']') != std::string::npos)
        {
            // Regular section
            size_t start = line.find('[');
            size_t end = line.find(']');
            if (start != std::string::npos && end != std::string::npos && end > start)
            {
                std::string sectionName = line.substr(start + 1, end - start - 1);
                addToken(lineNum, static_cast<int>(start), static_cast<int>(end - start + 1), SemanticTokenType::CLASS, 1);
                inDefineSection = false;
                currentSection = sectionName;
            }
        }
        
        // Highlight macro references (@name)
        for (const auto& [macroName, value] : defines)
        {
            (void)value;
            std::string pattern = "@" + macroName;
            size_t pos = 0;
            while ((pos = line.find(pattern, pos)) != std::string::npos)
            {
                // Make sure it's not @{...}
                if (pos + pattern.length() >= line.length() || line[pos + pattern.length()] != '{')
                {
                    addToken(lineNum, static_cast<int>(pos), static_cast<int>(pattern.length()), SemanticTokenType::MACRO, 4); // readonly
                }
                pos += pattern.length();
            }
        }
        
        // Highlight cross-section references @{Section.key}
        size_t atBrace = 0;
        while ((atBrace = line.find("@{", atBrace)) != std::string::npos)
        {
            size_t closeBrace = line.find("}", atBrace);
            if (closeBrace != std::string::npos)
            {
                addToken(lineNum, static_cast<int>(atBrace), static_cast<int>(closeBrace - atBrace + 1), SemanticTokenType::MACRO, 4);
            }
            atBrace++;
        }
        
        // Highlight environment variables ${VAR}
        size_t dollar = 0;
        while ((dollar = line.find("${", dollar)) != std::string::npos)
        {
            size_t closeBrace = line.find("}", dollar);
            if (closeBrace != std::string::npos)
            {
                addToken(lineNum, static_cast<int>(dollar), static_cast<int>(closeBrace - dollar + 1), SemanticTokenType::VARIABLE, 4);
            }
            dollar++;
        }
        
        // Highlight key names in sections
        if (!currentSection.empty() && !inDefineSection)
        {
            size_t equals = line.find('=');
            if (equals != std::string::npos)
            {
                std::string keyPart = line.substr(0, equals);
                size_t keyStart = keyPart.find_first_not_of(" \t");
                size_t keyEnd = keyPart.find_last_not_of(" \t");
                if (keyStart != std::string::npos && keyEnd != std::string::npos)
                {
                    addToken(lineNum, static_cast<int>(keyStart), static_cast<int>(keyEnd - keyStart + 1), SemanticTokenType::PROPERTY, 0);
                }
            }
        }
        
        // Highlight numbers
        std::istringstream lineStream(line);
        std::string word;
        int colPos = 0;
        while (lineStream >> word)
        {
            size_t wordPos = line.find(word, colPos);
            if (wordPos != std::string::npos)
            {
                // Check if it's a number
                bool isNumber = !word.empty() && (std::isdigit(word[0]) || (word[0] == '-' && word.length() > 1));
                if (isNumber)
                {
                    for (char c : word)
                    {
                        if (!std::isdigit(c) && c != '.' && c != '-' && c != 'e' && c != 'E')
                        {
                            isNumber = false;
                            break;
                        }
                    }
                    if (isNumber)
                    {
                        addToken(lineNum, static_cast<int>(wordPos), static_cast<int>(word.length()), SemanticTokenType::NUMBER, 0);
                    }
                }
                colPos = static_cast<int>(wordPos + word.length());
            }
        }
        
        // Highlight keywords
        if (line.find("true") != std::string::npos)
        {
            findAndAddTokens(line, lineNum, "true", SemanticTokenType::KEYWORD);
        }
        if (line.find("false") != std::string::npos)
        {
            findAndAddTokens(line, lineNum, "false", SemanticTokenType::KEYWORD);
        }
        
        lineNum++;
    }
}

json SemanticTokensProvider::encodeTokens()
{
    // Sort tokens by line, then by character
    std::sort(tokens.begin(), tokens.end(), [](const SemanticToken& a, const SemanticToken& b) {
        if (a.line != b.line) return a.line < b.line;
        return a.startChar < b.startChar;
    });
    
    // Delta-encode tokens
    json data = json::array();
    int prevLine = 0;
    int prevChar = 0;
    
    for (const auto& token : tokens)
    {
        int deltaLine = token.line - prevLine;
        int deltaStart = (deltaLine == 0) ? (token.startChar - prevChar) : token.startChar;
        
        data.push_back(deltaLine);
        data.push_back(deltaStart);
        data.push_back(token.length);
        data.push_back(static_cast<int>(token.type));
        data.push_back(token.modifiers);
        
        prevLine = token.line;
        prevChar = token.startChar;
    }
    
    return data;
}

json SemanticTokensProvider::getSemanticTokens(
    yini::Parser* parser,
    const std::string& content)
{
    extractTokens(parser, content);
    
    return {
        {"data", encodeTokens()}
    };
}

json SemanticTokensProvider::getSemanticTokensRange(
    yini::Parser* parser,
    const std::string& content,
    int startLine,
    int endLine)
{
    extractTokens(parser, content);
    
    // Filter tokens in range
    std::vector<SemanticToken> rangeTokens;
    for (const auto& token : tokens)
    {
        if (token.line >= startLine && token.line <= endLine)
        {
            rangeTokens.push_back(token);
        }
    }
    
    // Temporarily swap
    std::swap(tokens, rangeTokens);
    json result = {{"data", encodeTokens()}};
    std::swap(tokens, rangeTokens);
    
    return result;
}

} // namespace yini::lsp
