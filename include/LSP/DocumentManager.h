#ifndef YINI_DOCUMENT_MANAGER_H
#define YINI_DOCUMENT_MANAGER_H

#include "Interpreter.h"
#include "AST.h"
#include <string>
#include <map>
#include <memory>

namespace yini::lsp
{

struct Document
{
    std::string uri;
    std::string content;
    int version;
    std::shared_ptr<RootNode> ast;
    std::unique_ptr<Interpreter> interpreter;
    bool analyzed;
    std::string lastError;
    
    Document(const std::string& uri, const std::string& content, int version)
        : uri(uri), content(content), version(version), analyzed(false)
    {
    }
};

class DocumentManager
{
public:
    DocumentManager();
    
    // Document lifecycle
    void openDocument(const std::string& uri, 
                      const std::string& content, 
                      int version);
    
    void updateDocument(const std::string& uri, 
                        const std::string& content, 
                        int version);
    
    void closeDocument(const std::string& uri);
    
    // Document access
    Document* getDocument(const std::string& uri);
    
    // Get the interpreter for a document
    yini::Interpreter* getInterpreter(const std::string& uri);
    
    // Check if document exists
    bool hasDocument(const std::string& uri) const;
    
private:
    std::map<std::string, std::unique_ptr<Document>> documents;
    
    // Parse and interpret document content
    void analyzeDocument(Document* doc);
};

} // namespace yini::lsp

#endif // YINI_DOCUMENT_MANAGER_H
