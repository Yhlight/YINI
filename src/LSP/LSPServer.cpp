#include "LSP/LSPServer.h"
#include <iostream>

namespace yini::lsp
{

LSPServer::LSPServer()
    : documentManager(std::make_unique<DocumentManager>())
    , completionProvider(std::make_unique<CompletionProvider>())
    , hoverProvider(std::make_unique<HoverProvider>())
    , definitionProvider(std::make_unique<DefinitionProvider>())
    , symbolProvider(std::make_unique<SymbolProvider>())
    , initialized(false)
{
    // Register LSP method handlers
    rpcHandler.registerMethod("initialize", 
        [this](const json& params) { return handleInitialize(params); });
    
    rpcHandler.registerMethod("initialized", 
        [this](const json& params) { return handleInitialized(params); });
    
    rpcHandler.registerMethod("shutdown", 
        [this](const json& params) { return handleShutdown(params); });
    
    rpcHandler.registerMethod("exit", 
        [this](const json& params) { return handleExit(params); });
    
    // Text document synchronization
    rpcHandler.registerMethod("textDocument/didOpen", 
        [this](const json& params) { return handleTextDocumentDidOpen(params); });
    
    rpcHandler.registerMethod("textDocument/didChange", 
        [this](const json& params) { return handleTextDocumentDidChange(params); });
    
    rpcHandler.registerMethod("textDocument/didClose", 
        [this](const json& params) { return handleTextDocumentDidClose(params); });
    
    // Language features
    rpcHandler.registerMethod("textDocument/completion", 
        [this](const json& params) { return handleTextDocumentCompletion(params); });
    
    rpcHandler.registerMethod("textDocument/hover", 
        [this](const json& params) { return handleTextDocumentHover(params); });
    
    rpcHandler.registerMethod("textDocument/definition", 
        [this](const json& params) { return handleTextDocumentDefinition(params); });
    
    rpcHandler.registerMethod("textDocument/documentSymbol", 
        [this](const json& params) { return handleTextDocumentDocumentSymbol(params); });
}

void LSPServer::start()
{
    logMessage("YINI LSP Server starting...");
    rpcHandler.runLoop();
}

json LSPServer::handleInitialize(const json& /*params*/)
{
    initialized = true;
    
    // Return server capabilities
    return {
        {"capabilities", {
            {"textDocumentSync", {
                {"openClose", true},
                {"change", 1}  // Full sync
            }},
            {"completionProvider", {
                {"triggerCharacters", json::array({"@", "{", "."})}
            }},
            {"hoverProvider", true},
            {"definitionProvider", true},
            {"documentSymbolProvider", true}
        }},
        {"serverInfo", {
            {"name", "YINI Language Server"},
            {"version", "1.5.0"}
        }}
    };
}

json LSPServer::handleInitialized(const json& /*params*/)
{
    logMessage("Client initialized");
    return json::object();
}

json LSPServer::handleShutdown(const json& /*params*/)
{
    logMessage("Server shutting down");
    return nullptr;
}

json LSPServer::handleExit(const json& /*params*/)
{
    std::exit(0);
    return json::object();
}

json LSPServer::handleTextDocumentDidOpen(const json& params)
{
    auto textDocument = params["textDocument"];
    std::string uri = textDocument["uri"];
    std::string text = textDocument["text"];
    int version = textDocument["version"];
    
    documentManager->openDocument(uri, text, version);
    publishDiagnostics(uri);
    
    return json::object();
}

json LSPServer::handleTextDocumentDidChange(const json& params)
{
    auto textDocument = params["textDocument"];
    std::string uri = textDocument["uri"];
    int version = textDocument["version"];
    
    auto contentChanges = params["contentChanges"];
    if (!contentChanges.empty())
    {
        std::string text = contentChanges[0]["text"];
        documentManager->updateDocument(uri, text, version);
        publishDiagnostics(uri);
    }
    
    return json::object();
}

json LSPServer::handleTextDocumentDidClose(const json& params)
{
    auto textDocument = params["textDocument"];
    std::string uri = textDocument["uri"];
    
    documentManager->closeDocument(uri);
    
    return json::object();
}

json LSPServer::handleTextDocumentCompletion(const json& params)
{
    auto textDocument = params["textDocument"];
    std::string uri = textDocument["uri"];
    
    auto position = params["position"];
    int line = position["line"];
    int character = position["character"];
    
    auto doc = documentManager->getDocument(uri);
    if (!doc)
    {
        return json::array();
    }
    
    auto parser = documentManager->getParser(uri);
    
    yini::lsp::Position pos{line, character};
    return completionProvider->getCompletions(parser, doc->content, pos);
}

json LSPServer::handleTextDocumentHover(const json& params)
{
    auto textDocument = params["textDocument"];
    std::string uri = textDocument["uri"];
    
    auto position = params["position"];
    int line = position["line"];
    int character = position["character"];
    
    auto doc = documentManager->getDocument(uri);
    if (!doc)
    {
        return nullptr;
    }
    
    auto parser = documentManager->getParser(uri);
    
    yini::lsp::Position pos{line, character};
    return hoverProvider->getHover(parser, doc->content, pos);
}

json LSPServer::handleTextDocumentDefinition(const json& params)
{
    auto textDocument = params["textDocument"];
    std::string uri = textDocument["uri"];
    
    auto position = params["position"];
    int line = position["line"];
    int character = position["character"];
    
    auto doc = documentManager->getDocument(uri);
    if (!doc)
    {
        return nullptr;
    }
    
    auto parser = documentManager->getParser(uri);
    
    yini::lsp::Position pos{line, character};
    return definitionProvider->getDefinition(parser, doc->content, uri, pos);
}

json LSPServer::handleTextDocumentDocumentSymbol(const json& params)
{
    auto textDocument = params["textDocument"];
    std::string uri = textDocument["uri"];
    
    auto doc = documentManager->getDocument(uri);
    if (!doc)
    {
        return json::array();
    }
    
    auto parser = documentManager->getParser(uri);
    
    return symbolProvider->getDocumentSymbols(parser, doc->content);
}

json LSPServer::handleTextDocumentReferences(const json& params)
{
    auto textDocument = params["textDocument"];
    std::string uri = textDocument["uri"];
    
    auto position = params["position"];
    int line = position["line"];
    int character = position["character"];
    
    auto context = params["context"];
    bool includeDeclaration = context.value("includeDeclaration", true);
    
    auto doc = documentManager->getDocument(uri);
    if (!doc)
    {
        return json::array();
    }
    
    auto parser = documentManager->getParser(uri);
    
    yini::lsp::Position pos{line, character};
    return referenceProvider->findReferences(parser, doc->content, uri, pos, includeDeclaration);
}

json LSPServer::handleTextDocumentPrepareRename(const json& params)
{
    auto textDocument = params["textDocument"];
    std::string uri = textDocument["uri"];
    
    auto position = params["position"];
    int line = position["line"];
    int character = position["character"];
    
    auto doc = documentManager->getDocument(uri);
    if (!doc)
    {
        return nullptr;
    }
    
    auto parser = documentManager->getParser(uri);
    
    yini::lsp::Position pos{line, character};
    return renameProvider->prepareRename(parser, doc->content, pos);
}

json LSPServer::handleTextDocumentRename(const json& params)
{
    auto textDocument = params["textDocument"];
    std::string uri = textDocument["uri"];
    
    auto position = params["position"];
    int line = position["line"];
    int character = position["character"];
    
    std::string newName = params["newName"];
    
    auto doc = documentManager->getDocument(uri);
    if (!doc)
    {
        return nullptr;
    }
    
    auto parser = documentManager->getParser(uri);
    
    yini::lsp::Position pos{line, character};
    return renameProvider->rename(parser, doc->content, uri, pos, newName);
}

json LSPServer::handleTextDocumentFormatting(const json& params)
{
    auto textDocument = params["textDocument"];
    std::string uri = textDocument["uri"];
    
    auto options = params["options"];
    
    auto doc = documentManager->getDocument(uri);
    if (!doc)
    {
        return json::array();
    }
    
    FormattingOptions formattingOptions;
    formattingOptions.tabSize = options.value("tabSize", 4);
    formattingOptions.insertSpaces = options.value("insertSpaces", true);
    formattingOptions.trimTrailingWhitespace = options.value("trimTrailingWhitespace", true);
    formattingOptions.insertFinalNewline = options.value("insertFinalNewline", true);
    
    return formattingProvider->formatDocument(doc->content, formattingOptions);
}

json LSPServer::handleTextDocumentRangeFormatting(const json& params)
{
    auto textDocument = params["textDocument"];
    std::string uri = textDocument["uri"];
    
    auto range = params["range"];
    auto options = params["options"];
    
    auto doc = documentManager->getDocument(uri);
    if (!doc)
    {
        return json::array();
    }
    
    Range formatRange;
    formatRange.start.line = range["start"]["line"];
    formatRange.start.character = range["start"]["character"];
    formatRange.end.line = range["end"]["line"];
    formatRange.end.character = range["end"]["character"];
    
    FormattingOptions formattingOptions;
    formattingOptions.tabSize = options.value("tabSize", 4);
    formattingOptions.insertSpaces = options.value("insertSpaces", true);
    formattingOptions.trimTrailingWhitespace = options.value("trimTrailingWhitespace", true);
    formattingOptions.insertFinalNewline = options.value("insertFinalNewline", false);
    
    return formattingProvider->formatRange(doc->content, formatRange, formattingOptions);
}

void LSPServer::publishDiagnostics(const std::string& uri)
{
    auto doc = documentManager->getDocument(uri);
    if (!doc)
    {
        return;
    }
    
    json diagnostics = json::array();
    
    // If parsing failed, create diagnostic
    if (!doc->lastError.empty())
    {
        diagnostics.push_back({
            {"range", {
                {"start", {{"line", 0}, {"character", 0}}},
                {"end", {{"line", 0}, {"character", 0}}}
            }},
            {"severity", 1},  // Error
            {"source", "yini"},
            {"message", doc->lastError}
        });
    }
    
    // Publish diagnostics notification
    rpcHandler.sendNotification("textDocument/publishDiagnostics", {
        {"uri", uri},
        {"diagnostics", diagnostics}
    });
}

void LSPServer::logMessage(const std::string& message)
{
    rpcHandler.sendNotification("window/logMessage", {
        {"type", 3},  // Info
        {"message", message}
    });
}

} // namespace yini::lsp
