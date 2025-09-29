#include "YINI/Parser.hpp"
#include "YINI/YiniException.hpp"
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;

// Global map to store document contents, mapping URI to content.
std::map<std::string, std::string> documentContents;

void sendResponse(const json& response) {
    std::string responseStr = response.dump();
    std::cout << "Content-Length: " << responseStr.length() << "\r\n\r\n" << responseStr;
    std::cout.flush();
}

void publishDiagnostics(const std::string& uri, const std::string& content) {
    json diagnostics = json::array();
    try {
        YINI::YiniDocument doc;
        YINI::Parser parser(content, doc);
        parser.parse();
    } catch (const YINI::YiniException& e) {
        json diagnostic;
        diagnostic["range"] = {
            {"start", {{"line", e.getLine() - 1}, {"character", e.getColumn() - 1}}},
            {"end", {{"line", e.getLine() - 1}, {"character", e.getColumn()}}}
        };
        diagnostic["severity"] = 1; // Error
        diagnostic["source"] = "YINI LSP";
        diagnostic["message"] = e.what();
        diagnostics.push_back(diagnostic);
    }

    json params = {
        {"uri", uri},
        {"diagnostics", diagnostics}
    };

    json notification = {
        {"jsonrpc", "2.0"},
        {"method", "textDocument/publishDiagnostics"},
        {"params", params}
    };
    sendResponse(notification);
}


int main() {
    std::cerr << "YINI Language Server starting..." << std::endl;

    while (true) {
        // Read headers
        long long contentLength = -1;
        std::string line;
        while (std::getline(std::cin, line) && !line.empty() && line != "\r") {
            if (line.rfind("Content-Length: ", 0) == 0) {
                contentLength = std::stoll(line.substr(16));
            }
        }

        if (contentLength == -1) {
            continue;
        }

        // Read content
        std::vector<char> content(contentLength);
        std::cin.read(content.data(), contentLength);
        std::string contentStr(content.begin(), content.end());

        std::cerr << "Received message: " << contentStr << std::endl;

        try {
            json receivedJson = json::parse(contentStr);

            if (receivedJson.contains("method"))
            {
                std::string method = receivedJson["method"];

                if (method == "initialize") {
                    json response = {
                        {"jsonrpc", "2.0"},
                        {"id", receivedJson["id"]},
                        {"result", {
                            {"capabilities", {
                                {"textDocumentSync", {
                                    {"openClose", true},
                                    {"change", 1} // 1 = Full sync
                                }},
                                {"diagnosticProvider", {
                                    {"interFileDependencies", false},
                                    {"workspaceDiagnostics", false}
                            }},
                            {"hoverProvider", true}
                            }}
                        }}
                    };
                    sendResponse(response);
                    std::cerr << "Sent initialize response." << std::endl;
                } else if (method == "initialized") {
                    json message = {
                        {"jsonrpc", "2.0"},
                        {"method", "window/showMessage"},
                        {"params", {
                            {"type", 3}, // Info
                            {"message", "YINI Language Server connected!"}
                        }}
                    };
                    sendResponse(message);
                } else if (method == "textDocument/didOpen") {
                    std::string uri = receivedJson["params"]["textDocument"]["uri"];
                    std::string content = receivedJson["params"]["textDocument"]["text"];
                    documentContents[uri] = content;
                    publishDiagnostics(uri, content);
                } else if (method == "textDocument/didChange") {
                    std::string uri = receivedJson["params"]["textDocument"]["uri"];
                    std::string content = receivedJson["params"]["contentChanges"][0]["text"];
                    documentContents[uri] = content;
                    publishDiagnostics(uri, content);
                } else if (method == "exit") {
                    break;
                }
            }

        } catch (const json::parse_error& e) {
            std::cerr << "JSON Parse Error: " << e.what() << std::endl;
        }
    }

    std::cerr << "YINI Language Server shutting down." << std::endl;
    return 0;
}