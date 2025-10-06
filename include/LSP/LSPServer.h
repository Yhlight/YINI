#ifndef YINI_LSP_SERVER_H
#define YINI_LSP_SERVER_H

#include "LSP/JSONRPCHandler.h"
#include "LSP/DocumentManager.h"
#include "LSP/CompletionProvider.h"
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
    
    // Helper methods
    void publishDiagnostics(const std::string& uri);
    void logMessage(const std::string& message);
};

} // namespace yini::lsp

#endif // YINI_LSP_SERVER_H
