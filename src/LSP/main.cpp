#include "YINI/Parser.hpp"
#include "YINI/YiniException.hpp"
#include "YiniValueToString.hpp"
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <algorithm>

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

// A helper function to get the word/token at a specific position
std::string getWordAtPosition(const std::string& content, int line, int character) {
    std::stringstream ss(content);
    std::string lineContent;
    for (int i = 0; i <= line; ++i) {
        if (!std::getline(ss, lineContent)) return "";
    }

    size_t start = 0;
    size_t end = 0;

    // Find the start of the word
    for (int i = character; i >= 0; --i) {
        if (!isalnum(lineContent[i]) && lineContent[i] != '_') {
            start = i + 1;
            break;
        }
        start = i;
    }

    // Find the end of the word
    for (size_t i = character; i < lineContent.length(); ++i) {
        if (!isalnum(lineContent[i]) && lineContent[i] != '_') {
            end = i;
            break;
        }
        end = i + 1;
    }

    if (start < end) {
        return lineContent.substr(start, end - start);
    }
    return "";
}


int main() {
    std::cerr << "YINI Language Server starting..." << std::endl;

    while (true) {
        long long contentLength = -1;
        std::string line;
        while (std::getline(std::cin, line) && !line.empty() && line != "\r") {
            if (line.rfind("Content-Length: ", 0) == 0) {
                contentLength = std::stoll(line.substr(16));
            }
        }

        if (contentLength == -1) continue;

        std::vector<char> content(contentLength);
        std::cin.read(content.data(), contentLength);
        std::string contentStr(content.begin(), content.end());

        std::cerr << "Received message: " << contentStr << std::endl;

        try {
            json receivedJson = json::parse(contentStr);

            if (receivedJson.contains("method")) {
                std::string method = receivedJson["method"];

                if (method == "initialize") {
                    json response = {
                        {"jsonrpc", "2.0"},
                        {"id", receivedJson["id"]},
                        {"result", {
                            {"capabilities", {
                                {"textDocumentSync", 1}, // Full sync
                                {"hoverProvider", true},
                                {"completionProvider", {
                                    {"resolveProvider", false},
                                    {"triggerCharacters", {"@", "["}}
                                }}
                            }}
                        }}
                    };
                    sendResponse(response);
                } else if (method == "initialized") {
                    // Client is ready
                } else if (method == "textDocument/didOpen" || method == "textDocument/didChange") {
                    std::string uri = receivedJson["params"]["textDocument"]["uri"];
                    std::string docContent = (method == "textDocument/didOpen") ? receivedJson["params"]["textDocument"]["text"] : receivedJson["params"]["contentChanges"][0]["text"];
                    documentContents[uri] = docContent;
                    publishDiagnostics(uri, docContent);
                } else if (method == "textDocument/hover") {
                    std::string uri = receivedJson["params"]["textDocument"]["uri"];
                    int line = receivedJson["params"]["position"]["line"];
                    int character = receivedJson["params"]["position"]["character"];

                    YINI::YiniDocument doc;
                    YINI::Parser parser(documentContents[uri], doc);
                    parser.parse();
                    doc.resolveInheritance();

                    std::string word = getWordAtPosition(documentContents[uri], line, character);
                    json hoverContents = {{"language", "yini"}, {"value", ""}};

                    for (const auto& section : doc.getSections()) {
                        auto it = std::find_if(section.pairs.begin(), section.pairs.end(), [&](const auto& p) { return p.key == word; });
                        if (it != section.pairs.end()) {
                            hoverContents["value"] = YINI::valueToString(it->value);
                            break;
                        }
                    }

                    if (!hoverContents["value"].empty()) {
                        json response = {
                            {"jsonrpc", "2.0"},
                            {"id", receivedJson["id"]},
                            {"result", {
                                {"contents", hoverContents}
                            }}
                        };
                        sendResponse(response);
                    }
                } else if (method == "textDocument/completion") {
                    std::string uri = receivedJson["params"]["textDocument"]["uri"];

                    YINI::YiniDocument doc;
                    YINI::Parser parser(documentContents[uri], doc);
                    parser.parse();

                    json completionItems = json::array();
                    for (const auto& def : doc.getDefines()) {
                        completionItems.push_back({
                            {"label", def.first},
                            {"kind", 13}, // Variable
                            {"detail", "YINI Macro"}
                        });
                    }
                     for (const auto& sec : doc.getSections()) {
                        completionItems.push_back({
                            {"label", sec.name},
                            {"kind", 19}, // Class
                            {"detail", "YINI Section"}
                        });
                    }

                    json response = {
                        {"jsonrpc", "2.0"},
                        {"id", receivedJson["id"]},
                        {"result", completionItems}
                    };
                    sendResponse(response);
                }
                else if (method == "exit") {
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