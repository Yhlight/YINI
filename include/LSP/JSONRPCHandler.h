#ifndef YINI_JSONRPC_HANDLER_H
#define YINI_JSONRPC_HANDLER_H

#include <string>
#include <functional>
#include <map>
#include <nlohmann/json.hpp>

namespace yini::lsp
{

using json = nlohmann::json;
using MessageHandler = std::function<json(const json&)>;

class JSONRPCHandler
{
public:
    JSONRPCHandler();
    
    // Register method handler
    void registerMethod(const std::string& method, MessageHandler handler);
    
    // Process incoming message
    void processMessage(const std::string& message);
    
    // Send response
    void sendResponse(const json& response);
    
    // Send notification
    void sendNotification(const std::string& method, const json& params);
    
    // Send error
    void sendError(int id, int code, const std::string& message);
    
    // Main loop - read from stdin and process
    void runLoop();
    
private:
    std::map<std::string, MessageHandler> methodHandlers;
    
    json handleRequest(const json& request);
    void writeMessage(const json& message);
    std::string readMessage();
};

} // namespace yini::lsp

#endif // YINI_JSONRPC_HANDLER_H
