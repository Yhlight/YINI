#include "YINI/Parser.hpp"
#include "YINI/YiniException.hpp"
#include "YiniValueToString.hpp"
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>

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

std::string getWordAtPosition(const std::string& content, int line, int character) {
    std::stringstream ss(content);
    std::string lineContent;
    for (int i = 0; i <= line; ++i) {
        if (!std::getline(ss, lineContent)) return "";
    }

    size_t start = 0;
    size_t end = 0;

    for (int i = character; i >= 0; --i) {
        if (!isalnum(lineContent[i]) && lineContent[i] != '_') {
            start = i + 1;
            break;
        }
        start = i;
    }

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

std::string getLineAtPosition(const std::string& content, int line) {
    std::stringstream ss(content);
    std::string lineContent;
    for (int i = 0; i <= line; ++i) {
        if (!std::getline(ss, lineContent)) return "";
    }
    return lineContent;
}

// Helper to find the definition location of a symbol (section or macro)
json findDefinitionLocation(const std::string& content, const std::string& symbol) {
    std::stringstream ss(content);
    std::string line;
    int line_num = 0;

    std::string section_pattern = "[" + symbol + "]";
    std::string macro_pattern = symbol + " =";

    while (std::getline(ss, line)) {
        size_t section_pos = line.find(section_pattern);
        if (section_pos != std::string::npos) {
            return {
                {"line", line_num},
                {"character", section_pos + 1}
            };
        }

        size_t macro_pos = line.find(macro_pattern);
        if (macro_pos != std::string::npos && line.substr(0, macro_pos).find_first_not_of(" \t") == std::string::npos) {
             return {
                {"line", line_num},
                {"character", macro_pos}
            };
        }
        line_num++;
    }

    return nullptr;
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
                                {"textDocumentSync", 1},
                                {"hoverProvider", true},
                                {"definitionProvider", true},
                                {"completionProvider", {
                                    {"resolveProvider", false},
                                    {"triggerCharacters", {"@", ":", " "}}
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
                } else if (method == "textDocument/definition") {
                    std::string uri = receivedJson["params"]["textDocument"]["uri"];
                    int line = receivedJson["params"]["position"]["line"];
                    int character = receivedJson["params"]["position"]["character"];
                    std::string word = getWordAtPosition(documentContents[uri], line, character);

                    if (!word.empty()) {
                        json location = findDefinitionLocation(documentContents[uri], word);
                        if (!location.is_null()) {
                            json response = {
                                {"jsonrpc", "2.0"},
                                {"id", receivedJson["id"]},
                                {"result", {
                                    {"uri", uri},
                                    {"range", {
                                        {"start", location},
                                        {"end", location}
                                    }}
                                }}
                            };
                            sendResponse(response);
                        }
                    }
                }
                else if (method == "textDocument/completion") {
                    std::string uri = receivedJson["params"]["textDocument"]["uri"];
                    int line_num = receivedJson["params"]["position"]["line"];
                    int character = receivedJson["params"]["position"]["character"];

                    YINI::YiniDocument doc;
                    YINI::Parser parser(documentContents[uri], doc);
                    parser.parse();
                    doc.resolveInheritance();

                    json completionItems = json::array();
                    std::string lineContent = getLineAtPosition(documentContents[uri], line_num);

                    char triggerChar = 0;
                    if (character > 0) {
                        triggerChar = lineContent[character - 1];
                    }

                    if (triggerChar == '@') {
                        for (const auto& def : doc.getDefines()) {
                            completionItems.push_back({
                                {"label", def.first},
                                {"kind", 13},
                                {"detail", "YINI Macro"}
                            });
                        }
                    } else if (triggerChar == ':') {
                        for (const auto& sec : doc.getSections()) {
                            completionItems.push_back({
                                {"label", sec.name},
                                {"kind", 19},
                                {"detail", "YINI Section"}
                            });
                        }
                    } else {
                        size_t equalsPos = lineContent.find('=');
                        if (equalsPos != std::string::npos && equalsPos < (size_t)character) {
                            completionItems.push_back({{"label", "true"}, {"kind", 21}});
                            completionItems.push_back({{"label", "false"}, {"kind", 21}});
                        }
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