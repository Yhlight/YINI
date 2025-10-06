#ifndef YINI_LSP_SERVER_H
#define YINI_LSP_SERVER_H

#include "LSP/JSONRPCHandler.h"
#include "LSP/DocumentManager.h"
#include "LSP/CompletionProvider.h"
#include "LSP/HoverProvider.h"
#include "LSP/DefinitionProvider.h"
#include "LSP/SymbolProvider.h"
#include "LSP/ReferenceProvider.h"
#include "LSP/RenameProvider.h"
#include "LSP/FormattingProvider.h"
#include "LSP/SemanticTokensProvider.h"
#include "LSP/WorkspaceSymbolProvider.h"
#include <memory>

namespace yini::lsp
{

class LSPServer
{
public:
    LSPServer();
    
    // Start the LSP server
    void start();
    
private:
    JSONRPCHandler rpcHandler;
    std::unique_ptr<DocumentManager> documentManager;
    std::unique_ptr<CompletionProvider> completionProvider;
    std::unique_ptr<HoverProvider> hoverProvider;
    std::unique_ptr<DefinitionProvider> definitionProvider;
    std::unique_ptr<SymbolProvider> symbolProvider;
    std::unique_ptr<ReferenceProvider> referenceProvider;
    std::unique_ptr<RenameProvider> renameProvider;
    std::unique_ptr<FormattingProvider> formattingProvider;
    std::unique_ptr<SemanticTokensProvider> semanticTokensProvider;
    std::unique_ptr<WorkspaceSymbolProvider> workspaceSymbolProvider;
    bool initialized;
    
    // LSP method handlers
    json handleInitialize(const json& params);
    json handleInitialized(const json& params);
    json handleShutdown(const json& params);
    json handleExit(const json& params);
    
    // Text document synchronization
    json handleTextDocumentDidOpen(const json& params);
    json handleTextDocumentDidChange(const json& params);
    json handleTextDocumentDidClose(const json& params);
    
    // Language features
    json handleTextDocumentCompletion(const json& params);
    json handleTextDocumentHover(const json& params);
    json handleTextDocumentDefinition(const json& params);
    json handleTextDocumentDocumentSymbol(const json& params);
    json handleTextDocumentReferences(const json& params);
    json handleTextDocumentPrepareRename(const json& params);
    json handleTextDocumentRename(const json& params);
    json handleTextDocumentFormatting(const json& params);
    json handleTextDocumentRangeFormatting(const json& params);
    json handleTextDocumentSemanticTokensFull(const json& params);
    json handleTextDocumentSemanticTokensRange(const json& params);
    json handleWorkspaceSymbol(const json& params);
    
    // Helper methods
    void publishDiagnostics(const std::string& uri);
    void logMessage(const std::string& message);
};

} // namespace yini::lsp

#endif // YINI_LSP_SERVER_H
