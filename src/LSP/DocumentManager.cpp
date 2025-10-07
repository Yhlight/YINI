#include "LSP/DocumentManager.h"
#include "Lexer.h"

namespace yini::lsp
{

DocumentManager::DocumentManager()
{
}

void DocumentManager::openDocument(const std::string& uri, 
                                   const std::string& content, 
                                   int version)
{
    auto doc = std::make_unique<Document>(uri, content, version);
    parseDocument(doc.get());
    documents[uri] = std::move(doc);
}

void DocumentManager::updateDocument(const std::string& uri, 
                                     const std::string& content, 
                                     int version)
{
    auto it = documents.find(uri);
    if (it != documents.end())
    {
        it->second->content = content;
        it->second->version = version;
        it->second->parsed = false;
        parseDocument(it->second.get());
    }
}

void DocumentManager::closeDocument(const std::string& uri)
{
    documents.erase(uri);
}

Document* DocumentManager::getDocument(const std::string& uri)
{
    auto it = documents.find(uri);
    if (it != documents.end())
    {
        return it->second.get();
    }
    return nullptr;
}

yini::Parser* DocumentManager::getParser(const std::string& uri)
{
    auto doc = getDocument(uri);
    if (doc && doc->parsed && doc->parser)
    {
        return doc->parser.get();
    }
    return nullptr;
}

bool DocumentManager::hasDocument(const std::string& uri) const
{
    return documents.find(uri) != documents.end();
}

void DocumentManager::parseDocument(Document* doc)
{
    if (!doc)
    {
        return;
    }
    
    try
    {
        // Create parser with document content
        doc->parser = std::make_unique<yini::Parser>(doc->content);
        
        // Parse
        bool success = doc->parser->parse();
        doc->parsed = true;
        
        if (!success)
        {
            doc->lastError = doc->parser->getLastError();
        }
        else
        {
            doc->lastError = "";
        }
    }
    catch (const std::exception& e)
    {
        doc->parsed = false;
        doc->lastError = e.what();
    }
}

} // namespace yini::lsp
