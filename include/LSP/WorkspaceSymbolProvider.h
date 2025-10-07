#ifndef YINI_WORKSPACE_SYMBOL_PROVIDER_H
#define YINI_WORKSPACE_SYMBOL_PROVIDER_H

#include "Interpreter.h"
#include "AST.h"
#include "LSP/LSPTypes.h"
#include <nlohmann/json.hpp>
#include <string>
#include <map>
#include <memory>

namespace yini::lsp
{

using json = nlohmann::json;

struct WorkspaceFile
{
    std::string uri;
    std::string content;
    std::shared_ptr<RootNode> ast;
    std::unique_ptr<Interpreter> interpreter;
    bool analyzed = false;
};

class WorkspaceSymbolProvider
{
public:
    WorkspaceSymbolProvider();
    
    // Add file to workspace
    void addFile(const std::string& uri, const std::string& content);
    
    // Remove file from workspace
    void removeFile(const std::string& uri);
    
    // Update file in workspace
    void updateFile(const std::string& uri, const std::string& content);
    
    // Search symbols across workspace
    json searchSymbols(const std::string& query);
    
private:
    std::map<std::string, std::unique_ptr<WorkspaceFile>> files;
    
    // Analyze file if needed
    void analyzeFile(WorkspaceFile* file);
    
    // Check if symbol name matches query
    bool matchesQuery(const std::string& symbolName, const std::string& query);
    
    // Create symbol information
    json makeSymbolInformation(
        const std::string& name,
        int kind,
        const std::string& uri,
        int line,
        int character,
        const std::string& containerName = ""
    );
};

} // namespace yini::lsp

#endif // YINI_WORKSPACE_SYMBOL_PROVIDER_H
