#include "LSP/WorkspaceSymbolProvider.h"
#include "Parser.h"
#include "Interpreter.h"
#include <algorithm>
#include <cctype>

namespace yini::lsp
{

WorkspaceSymbolProvider::WorkspaceSymbolProvider() {}

void WorkspaceSymbolProvider::analyzeFile(WorkspaceFile* file)
{
    if (!file || file->analyzed)
    {
        return;
    }

    try
    {
        Parser parser(file->content);
        file->ast = parser.parse();
        if (parser.hasError())
        {
            file->analyzed = false;
            return;
        }

        file->interpreter = std::make_unique<Interpreter>();
        file->analyzed = file->interpreter->interpret(*file->ast);
    }
    catch (...)
    {
        file->analyzed = false;
    }
}

void WorkspaceSymbolProvider::addFile(const std::string& uri, const std::string& content)
{
    auto file = std::make_unique<WorkspaceFile>();
    file->uri = uri;
    file->content = content;
    
    analyzeFile(file.get());
    files[uri] = std::move(file);
}

void WorkspaceSymbolProvider::removeFile(const std::string& uri)
{
    files.erase(uri);
}

void WorkspaceSymbolProvider::updateFile(const std::string& uri, const std::string& content)
{
    auto it = files.find(uri);
    if (it != files.end())
    {
        it->second->content = content;
        it->second->analyzed = false;
        analyzeFile(it->second.get());
    }
    else
    {
        addFile(uri, content);
    }
}

bool WorkspaceSymbolProvider::matchesQuery(const std::string& symbolName, const std::string& query)
{
    if (query.empty())
    {
        return true;
    }

    std::string lowerSymbol = symbolName;
    std::string lowerQuery = query;
    std::transform(lowerSymbol.begin(), lowerSymbol.end(), lowerSymbol.begin(), ::tolower);
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);

    return lowerSymbol.find(lowerQuery) != std::string::npos;
}

json WorkspaceSymbolProvider::makeSymbolInformation(
    const std::string& name,
    int kind,
    const std::string& uri,
    int line,
    int character,
    const std::string& containerName)
{
    json symbol = {
        {"name", name},
        {"kind", kind},
        {"location", {
            {"uri", uri},
            {"range", {
                {"start", {{"line", line}, {"character", character}}},
                {"end", {{"line", line}, {"character", character + static_cast<int>(name.length())}}}
            }}
        }}
    };
    if (!containerName.empty())
    {
        symbol["containerName"] = containerName;
    }
    return symbol;
}

json WorkspaceSymbolProvider::searchSymbols(const std::string& query)
{
    json symbols = json::array();

    for (const auto& [uri, file] : files)
    {
        if (!file->analyzed || !file->interpreter)
        {
            continue;
        }

        // Search in defines
        const auto& defines = file->interpreter->getDefines();
        for (const auto& [name, value] : defines)
        {
            (void)value;
            if (matchesQuery(name, query))
            {
                symbols.push_back(makeSymbolInformation(name, 13, uri, 0, 0, "[#define]"));
            }
        }

        // Search in sections
        const auto& sections = file->interpreter->getSections();
        for (const auto& [sectionName, section] : sections)
        {
            if (matchesQuery(sectionName, query))
            {
                symbols.push_back(makeSymbolInformation(sectionName, 5, uri, 0, 0));
            }
            
            // Search in keys
            for (const auto& [key, value] : section.entries)
            {
                (void)value;
                if (matchesQuery(key, query))
                {
                    symbols.push_back(makeSymbolInformation(key, 7, uri, 0, 0, "[" + sectionName + "]"));
                }
            }
        }
    }
    
    return symbols;
}

} // namespace yini::lsp