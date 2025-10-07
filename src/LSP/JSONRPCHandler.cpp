#include "LSP/JSONRPCHandler.h"
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace yini::lsp
{

JSONRPCHandler::JSONRPCHandler()
{
}

void JSONRPCHandler::registerMethod(const std::string& method, MessageHandler handler)
{
    methodHandlers[method] = handler;
}

std::string JSONRPCHandler::readMessage()
{
    std::string line;
    int contentLength = 0;
    
    // Read headers
    while (std::getline(std::cin, line))
    {
        if (line == "\r" || line.empty())
        {
            break; // End of headers
        }
        
        // Parse Content-Length header
        if (line.find("Content-Length: ") == 0)
        {
            contentLength = std::stoi(line.substr(16));
        }
    }
    
    if (contentLength == 0)
    {
        return "";
    }
    
    // Read content
    std::string content(contentLength, '\0');
    std::cin.read(&content[0], contentLength);
    
    return content;
}

void JSONRPCHandler::writeMessage(const json& message)
{
    std::string content = message.dump();
    
    std::cout << "Content-Length: " << content.length() << "\r\n\r\n";
    std::cout << content << std::flush;
}

json JSONRPCHandler::handleRequest(const json& request)
{
    try
    {
        std::string method = request["method"];
        auto it = methodHandlers.find(method);
        
        if (it == methodHandlers.end())
        {
            // Method not found
            return {
                {"jsonrpc", "2.0"},
                {"id", request.value("id", nullptr)},
                {"error", {
                    {"code", -32601},
                    {"message", "Method not found: " + method}
                }}
            };
        }
        
        // Call handler
        json result = it->second(request.value("params", json::object()));
        
        // Return response
        if (request.contains("id"))
        {
            return {
                {"jsonrpc", "2.0"},
                {"id", request["id"]},
                {"result", result}
            };
        }
        
        // Notification (no response needed)
        return json::object();
    }
    catch (const std::exception& e)
    {
        return {
            {"jsonrpc", "2.0"},
            {"id", request.value("id", nullptr)},
            {"error", {
                {"code", -32603},
                {"message", std::string("Internal error: ") + e.what()}
            }}
        };
    }
}

void JSONRPCHandler::processMessage(const std::string& message)
{
    if (message.empty())
    {
        return;
    }
    
    try
    {
        json request = json::parse(message);
        json response = handleRequest(request);
        
        // Send response if not empty
        if (!response.empty() && response.contains("id"))
        {
            sendResponse(response);
        }
    }
    catch (const json::parse_error& e)
    {
        // Parse error
        json error_response = {
            {"jsonrpc", "2.0"},
            {"error", {
                {"code", -32700},
                {"message", "Parse error: " + std::string(e.what())}
            }}
        };
        sendResponse(error_response);
    }
}

void JSONRPCHandler::sendResponse(const json& response)
{
    writeMessage(response);
}

void JSONRPCHandler::sendNotification(const std::string& method, const json& params)
{
    json notification = {
        {"jsonrpc", "2.0"},
        {"method", method},
        {"params", params}
    };
    
    writeMessage(notification);
}

void JSONRPCHandler::sendError(int id, int code, const std::string& message)
{
    json error_response = {
        {"jsonrpc", "2.0"},
        {"id", id},
        {"error", {
            {"code", code},
            {"message", message}
        }}
    };
    
    sendResponse(error_response);
}

void JSONRPCHandler::runLoop()
{
    while (true)
    {
        std::string message = readMessage();
        
        if (std::cin.eof())
        {
            break; // Connection closed
        }
        
        processMessage(message);
    }
}

} // namespace yini::lsp
