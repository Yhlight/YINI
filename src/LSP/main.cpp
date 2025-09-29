#include "YINI/Parser.hpp"
#include "YINI/YiniException.hpp"
#include "YINI/YiniFormatter.hpp"
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <memory>
#include <sstream>

using json = nlohmann::json;

// Global maps to store document state.
std::map<std::string, std::string> documentContents;
std::map<std::string, std::shared_ptr<YINI::YiniDocument>> documentAsts;

void sendResponse(const json& response) {
    std::string responseStr = response.dump();
    std::cout << "Content-Length: " << responseStr.length() << "\r\n\r\n" << responseStr;
    std::cout.flush();
}

void publishDiagnostics(const std::string& uri, const std::string& content) {
    json diagnostics = json::array();
    auto doc = std::make_shared<YINI::YiniDocument>();
    documentAsts[uri] = doc; // Store the (potentially empty) AST

    try {
        YINI::Parser parser(content, *doc);
        parser.parse();
        doc->resolveInheritance();
    } catch (const YINI::YiniParsingException& e) {
        for (const auto& err : e.getErrors()) {
            json diagnostic;
            diagnostic["range"] = {
                {"start", {{"line", err.line - 1}, {"character", err.column - 1}}},
                {"end", {{"line", err.line - 1}, {"character", err.column}}}
            };
            diagnostic["severity"] = 1; // Error
            diagnostic["source"] = "YINI LSP";
            diagnostic["message"] = err.message;
            diagnostics.push_back(diagnostic);
        }
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
                                    {"change", 1}
                                }},
                                {"diagnosticProvider", {
                                    {"interFileDependencies", false},
                                    {"workspaceDiagnostics", false}
                                }},
                                {"hoverProvider", true},
                                {"completionProvider", {
                                    {"resolveProvider", false},
                                    {"triggerCharacters", {"@"}}
                                }},
                                {"definitionProvider", true},
                                {"documentFormattingProvider", true}
                            }}
                        }}
                    };
                    sendResponse(response);
                } else if (method == "initialized") {
                    // ...
                } else if (method == "textDocument/didOpen" || method == "textDocument/didChange") {
                    std::string uri = receivedJson["params"]["textDocument"]["uri"];
                    std::string content = (method == "textDocument/didOpen")
                                        ? receivedJson["params"]["textDocument"]["text"]
                                        : receivedJson["params"]["contentChanges"][0]["text"];
                    documentContents[uri] = content;
                    publishDiagnostics(uri, content);
                } else if (method == "textDocument/hover") {
                    std::string uri = receivedJson["params"]["textDocument"]["uri"];
                    int line_num = receivedJson["params"]["position"]["line"];
                    int char_num = receivedJson["params"]["position"]["character"];

                    std::string hover_content = "";
                    if (documentContents.count(uri) && documentAsts.count(uri)) {
                        std::string content = documentContents.at(uri);
                        auto doc = documentAsts.at(uri);

                        std::stringstream ss(content);
                        std::string line;
                        std::vector<std::string> lines;
                        while(std::getline(ss, line, '\n')) lines.push_back(line);

                        if (line_num < lines.size()) {
                            std::string hover_line = lines[line_num];
                            int start = char_num;
                            while (start > 0 && (isalnum(hover_line[start-1]) || hover_line[start-1] == '_')) start--;
                            int end = char_num;
                            while (end < hover_line.length() && (isalnum(hover_line[end]) || hover_line[end] == '_')) end++;

                            std::string word = hover_line.substr(start, end - start);

                            if (!word.empty()) {
                                if (start > 0 && hover_line[start-1] == '@') {
                                    YINI::YiniDefine define;
                                    if(doc->getDefine(word, define)) {
                                        hover_content = "(macro) " + word + " = " + YINI::YiniFormatter::format(define.value);
                                    }
                                } else {
                                    std::string section_name;
                                    for(int i = line_num; i >= 0; --i) {
                                        std::string l = lines[i];
                                        l.erase(0, l.find_first_not_of(" \t"));
                                        if (l.rfind('[') == 0 && l.find(']') != std::string::npos) {
                                            section_name = l.substr(1, l.find(']') - 1);
                                            size_t colon_pos = section_name.find(':');
                                            if (colon_pos != std::string::npos) {
                                                section_name = section_name.substr(0, colon_pos);
                                                section_name.erase(section_name.find_last_not_of(" \t") + 1);
                                            }
                                            break;
                                        }
                                    }
                                    if (!section_name.empty()) {
                                        const auto* section = doc->findSection(section_name);
                                        if (section) {
                                            auto it = std::find_if(section->pairs.begin(), section->pairs.end(), [&](const auto& p){ return p.key == word; });
                                            if (it != section->pairs.end()) {
                                                hover_content = (it->is_dynamic ? "(dynamic) " : "") + it->key + " = " + YINI::YiniFormatter::format(it->value);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    json response;
                    if (!hover_content.empty()) {
                        json contents;
                        contents["kind"] = "markdown";
                        contents["value"] = "```yini\n" + hover_content + "\n```";

                        json result;
                        result["contents"] = contents;

                        response = {
                            {"jsonrpc", "2.0"},
                            {"id", receivedJson["id"]},
                            {"result", result}
                        };
                    } else {
                        response = { {"jsonrpc", "2.0"}, {"id", receivedJson["id"]}, {"result", nullptr} };
                    }
                    sendResponse(response);
                } else if (method == "textDocument/completion") {
                    std::string uri = receivedJson["params"]["textDocument"]["uri"];
                    json items = json::array();

                    if (documentAsts.count(uri)) {
                        auto doc = documentAsts.at(uri);
                        const auto& defines = doc->getDefines();
                        for (const auto& [key, define] : defines) {
                            json item;
                            item["label"] = key;
                            item["kind"] = 13; // Variable
                            item["detail"] = YINI::YiniFormatter::format(define.value);
                            item["insertText"] = key;
                            items.push_back(item);
                        }
                    }

                    json response = {
                        {"jsonrpc", "2.0"},
                        {"id", receivedJson["id"]},
                        {"result", {{"isIncomplete", false}, {"items", items}}}
                    };
                    sendResponse(response);
                } else if (method == "textDocument/definition") {
                    std::string uri = receivedJson["params"]["textDocument"]["uri"];
                    int line_num = receivedJson["params"]["position"]["line"];
                    int char_num = receivedJson["params"]["position"]["character"];

                    json location = nullptr;

                    if (documentContents.count(uri) && documentAsts.count(uri)) {
                        std::string content = documentContents.at(uri);
                        auto doc = documentAsts.at(uri);

                        std::stringstream ss(content);
                        std::string line;
                        std::vector<std::string> lines;
                        while(std::getline(ss, line, '\n')) lines.push_back(line);

                        if (line_num < lines.size()) {
                            std::string hover_line = lines[line_num];
                            int start = char_num;
                            while (start > 0 && (isalnum(hover_line[start-1]) || hover_line[start-1] == '_')) start--;
                            int end = char_num;
                            while (end < hover_line.length() && (isalnum(hover_line[end]) || hover_line[end] == '_')) end++;

                            std::string word = hover_line.substr(start, end - start);

                            if (!word.empty() && start > 0 && hover_line[start-1] == '@') {
                                YINI::YiniDefine define;
                                if (doc->getDefine(word, define)) {
                                    location = {
                                        {"uri", uri},
                                        {"range", {
                                            {"start", {{"line", define.location.line - 1}, {"character", define.location.column - 1}}},
                                            {"end", {{"line", define.location.line - 1}, {"character", define.location.column}}}
                                        }}
                                    };
                                }
                            }
                        }
                    }

                    json response = {
                        {"jsonrpc", "2.0"},
                        {"id", receivedJson["id"]},
                        {"result", location}
                    };
                    sendResponse(response);
                } else if (method == "textDocument/formatting") {
                    std::string uri = receivedJson["params"]["textDocument"]["uri"];
                    json result = nullptr;

                    if (documentAsts.count(uri)) {
                        auto doc = documentAsts.at(uri);
                        std::string formatted_content = YINI::YiniFormatter::formatDocument(*doc);

                        // To replace the whole document, we need to know its full range.
                        // We can get this from the original content.
                        std::string original_content = documentContents.at(uri);
                        std::stringstream ss(original_content);
                        int line_count = 0;
                        std::string line;
                        while(std::getline(ss, line, '\n')) {
                            line_count++;
                        }

                        json text_edit;
                        text_edit["range"] = {
                            {"start", {{"line", 0}, {"character", 0}}},
                            {"end", {{"line", line_count}, {"character", 0}}}
                        };
                        text_edit["newText"] = formatted_content;

                        result = json::array({text_edit});
                    }

                    json response = {
                        {"jsonrpc", "2.0"},
                        {"id", receivedJson["id"]},
                        {"result", result}
                    };
                    sendResponse(response);
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