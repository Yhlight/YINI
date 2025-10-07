#include "LSP/DocumentManager.h"
#include "Parser.h"
#include "Interpreter.h"

namespace yini::lsp
{

DocumentManager::DocumentManager() {}

void DocumentManager::openDocument(const std::string& uri, const std::string& content, int version)
{
    auto doc = std::make_unique<Document>(uri, content, version);
    analyzeDocument(doc.get());
    documents[uri] = std::move(doc);
}

void DocumentManager::updateDocument(const std::string& uri, const std::string& content, int version)
{
    auto it = documents.find(uri);
    if (it != documents.end())
    {
        it->second->content = content;
        it->second->version = version;
        it->second->analyzed = false;
        analyzeDocument(it->second.get());
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

yini::Interpreter* DocumentManager::getInterpreter(const std::string& uri)
{
    auto doc = getDocument(uri);
    if (doc && doc->analyzed && doc->interpreter)
    {
        return doc->interpreter.get();
    }
    return nullptr;
}

bool DocumentManager::hasDocument(const std::string& uri) const
{
    return documents.find(uri) != documents.end();
}

void DocumentManager::analyzeDocument(Document* doc)
{
    if (!doc)
    {
        return;
    }

    try
    {
        // Stage 1: Parsing
        yini::Parser parser(doc->content);
        doc->ast = parser.parse();

        if (parser.hasError())
        {
            doc->analyzed = false;
            doc->lastError = parser.getLastError();
            return;
        }

        // Stage 2: Interpretation
        doc->interpreter = std::make_unique<yini::Interpreter>();
        if (!doc->interpreter->interpret(*doc->ast))
        {
            doc->analyzed = false;
            doc->lastError = doc->interpreter->getLastError();
            return;
        }

        doc->analyzed = true;
        doc->lastError = "";
    }
    catch (const std::exception& e)
    {
        doc->analyzed = false;
        doc->lastError = e.what();
    }
}

} // namespace yini::lsp
