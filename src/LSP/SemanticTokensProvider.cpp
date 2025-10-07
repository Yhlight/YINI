#include "LSP/SemanticTokensProvider.h"
#include <sstream>
#include <algorithm>

namespace yini::lsp
{

SemanticTokensProvider::SemanticTokensProvider() {}

json SemanticTokensProvider::getLegend()
{
    return {
        {"tokenTypes", json::array({
            "namespace", "class", "enum", "interface", "struct",
            "typeParameter", "parameter", "variable", "property", "enumMember",
            "decorator", "event", "function", "method", "macro",
            "label", "comment", "string", "keyword", "number",
            "regexp", "operator"
        })},
        {"tokenModifiers", json::array({
            "declaration", "definition", "readonly", "static", "deprecated",
            "abstract", "async", "modification", "documentation", "defaultLibrary"
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

void SemanticTokensProvider::extractTokens(yini::Interpreter* interpreter, Document* document)
{
    tokens.clear();
    if (!interpreter || !document)
    {
        return;
    }

    const std::string& content = document->content;
    std::istringstream stream(content);
    std::string line;
    int lineNum = 0;
    bool inDefineSection = false;
    std::string currentSection;

    const auto& defines = interpreter->getDefines();

    while (std::getline(stream, line))
    {
        if (line.find("[#define]") != std::string::npos)
        {
            addToken(lineNum, static_cast<int>(line.find("[#define]")), 9, SemanticTokenType::NAMESPACE, 1);
            inDefineSection = true;
            currentSection = "#define";
        }
        else if (line.find("[#include]") != std::string::npos)
        {
            addToken(lineNum, static_cast<int>(line.find("[#include]")), 10, SemanticTokenType::NAMESPACE, 1);
            inDefineSection = false;
            currentSection = "#include";
        }
        else if (line.find("[#schema]") != std::string::npos)
        {
            addToken(lineNum, static_cast<int>(line.find("[#schema]")), 9, SemanticTokenType::NAMESPACE, 1);
            inDefineSection = false;
            currentSection = "#schema";
        }
        else if (line.find('[') != std::string::npos && line.find(']') != std::string::npos)
        {
            size_t start = line.find('[');
            size_t end = line.find(']');
            if (end > start)
            {
                addToken(lineNum, static_cast<int>(start), static_cast<int>(end - start + 1), SemanticTokenType::CLASS, 1);
                inDefineSection = false;
                currentSection = line.substr(start + 1, end - start - 1);
            }
        }

        for (const auto& [macroName, value] : defines)
        {
            (void)value;
            std::string pattern = "@" + macroName;
            size_t pos = 0;
            while ((pos = line.find(pattern, pos)) != std::string::npos)
            {
                if (pos + pattern.length() >= line.length() || line[pos + pattern.length()] != '{')
                {
                    addToken(lineNum, static_cast<int>(pos), static_cast<int>(pattern.length()), SemanticTokenType::MACRO, 4);
                }
                pos += pattern.length();
            }
        }

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

        if (!currentSection.empty() && !inDefineSection)
        {
            size_t equals = line.find('=');
            if (equals != std::string::npos)
            {
                std::string keyPart = line.substr(0, equals);
                size_t keyStart = keyPart.find_first_not_of(" \t");
                if (keyStart != std::string::npos)
                {
                    size_t keyEnd = keyPart.find_last_not_of(" \t");
                    addToken(lineNum, static_cast<int>(keyStart), static_cast<int>(keyEnd - keyStart + 1), SemanticTokenType::PROPERTY, 0);
                }
            }
        }
        
        findAndAddTokens(line, lineNum, "true", SemanticTokenType::KEYWORD);
        findAndAddTokens(line, lineNum, "false", SemanticTokenType::KEYWORD);

        lineNum++;
    }
}

json SemanticTokensProvider::encodeTokens()
{
    std::sort(tokens.begin(), tokens.end(), [](const SemanticToken& a, const SemanticToken& b) {
        if (a.line != b.line) return a.line < b.line;
        return a.startChar < b.startChar;
    });

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
    yini::Interpreter* interpreter,
    Document* document)
{
    extractTokens(interpreter, document);
    return {{"data", encodeTokens()}};
}

json SemanticTokensProvider::getSemanticTokensRange(
    yini::Interpreter* interpreter,
    Document* document,
    int startLine,
    int endLine)
{
    extractTokens(interpreter, document);
    
    std::vector<SemanticToken> rangeTokens;
    for (const auto& token : tokens)
    {
        if (token.line >= startLine && token.line <= endLine)
        {
            rangeTokens.push_back(token);
        }
    }
    
    std::swap(tokens, rangeTokens);
    json result = {{"data", encodeTokens()}};
    std::swap(tokens, rangeTokens);
    
    return result;
}

} // namespace yini::lsp