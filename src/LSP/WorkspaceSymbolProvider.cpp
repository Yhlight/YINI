#include "LSP/WorkspaceSymbolProvider.h"
#include <algorithm>
#include <cctype>

namespace yini::lsp
{

WorkspaceSymbolProvider::WorkspaceSymbolProvider()
{
}

void WorkspaceSymbolProvider::parseFile(WorkspaceFile* file)
{
    if (!file || file->parsed)
    {
        return;
    }
    
    try
    {
        file->parser = std::make_unique<yini::Parser>(file->content);
        file->parsed = file->parser->parse();
    }
    catch (...)
    {
        file->parsed = false;
    }
}

void WorkspaceSymbolProvider::addFile(const std::string& uri, const std::string& content)
{
    auto file = std::make_unique<WorkspaceFile>();
    file->uri = uri;
    file->content = content;
    file->parsed = false;
    
    parseFile(file.get());
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
        it->second->parsed = false;
        parseFile(it->second.get());
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
    
    // Case-insensitive fuzzy match
    std::string lowerSymbol = symbolName;
    std::string lowerQuery = query;
    
    std::transform(lowerSymbol.begin(), lowerSymbol.end(), lowerSymbol.begin(), ::tolower);
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
    
    // Simple contains check
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
        if (!file->parsed || !file->parser)
        {
            continue;
        }
        
        // Search in defines
        const auto& defines = file->parser->getDefines();
        for (const auto& [name, value] : defines)
        {
            (void)value;
            if (matchesQuery(name, query))
            {
                symbols.push_back(makeSymbolInformation(
                    name,
                    13, // Variable
                    uri,
                    0, // Line number (simplified)
                    0,
                    "[#define]"
                ));
            }
        }
        
        // Search in sections
        const auto& sections = file->parser->getSections();
        for (const auto& [sectionName, section] : sections)
        {
            if (matchesQuery(sectionName, query))
            {
                symbols.push_back(makeSymbolInformation(
                    sectionName,
                    5, // Class
                    uri,
                    0,
                    0
                ));
            }
            
            // Search in keys
            for (const auto& [key, value] : section.entries)
            {
                (void)value;
                if (matchesQuery(key, query))
                {
                    symbols.push_back(makeSymbolInformation(
                        key,
                        7, // Property
                        uri,
                        0,
                        0,
                        "[" + sectionName + "]"
                    ));
                }
            }
        }
    }
    
    return symbols;
}

} // namespace yini::lsp
