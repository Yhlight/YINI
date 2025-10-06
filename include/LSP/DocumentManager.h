#ifndef YINI_DOCUMENT_MANAGER_H
#define YINI_DOCUMENT_MANAGER_H

#include "Parser.h"
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
    std::unique_ptr<yini::Parser> parser;
    bool parsed;
    std::string lastError;
    
    Document(const std::string& uri, const std::string& content, int version)
        : uri(uri), content(content), version(version), parsed(false)
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
    
    // Get parsed Parser (trigger parse if needed)
    yini::Parser* getParser(const std::string& uri);
    
    // Check if document exists
    bool hasDocument(const std::string& uri) const;
    
private:
    std::map<std::string, std::unique_ptr<Document>> documents;
    
    // Parse document content
    void parseDocument(Document* doc);
};

} // namespace yini::lsp

#endif // YINI_DOCUMENT_MANAGER_H
