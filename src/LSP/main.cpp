#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "nlohmann/json.hpp"
#include "Core/YiniManager.h"
#include "Core/YiniException.h"

using json = nlohmann::json;

namespace {
    void log_message(const std::string& message) {
        std::cerr << "[LSP Log] " << message << std::endl;
    }

    void send_response(const json& response) {
        std::string response_str = response.dump();
        std::cout << "Content-Length: " << response_str.length() << "\r\n\r\n" << response_str;
        std::cout.flush();
        log_message("Sent: " + response_str);
    }

    void send_notification(const json& notification) {
        send_response(notification);
    }
}

class LspServer {
public:
    void run() {
        log_message("YINI Language Server started.");
        while (std::cin) {
            std::string line;
            long content_length = 0;

            while (std::getline(std::cin, line) && !line.empty() && line != "\r") {
                if (line.rfind("Content-Length: ", 0) == 0) {
                    content_length = std::stol(line.substr(16));
                }
            }

            if (content_length == 0) continue;

            std::vector<char> buffer(content_length);
            std::cin.read(buffer.data(), content_length);
            std::string content(buffer.begin(), buffer.end());

            log_message("Received: " + content);
            handle_message(content);
        }
        log_message("YINI Language Server shutting down.");
    }

private:
    void handle_message(const std::string& content) {
        try {
            json msg = json::parse(content);
            if (msg.contains("method")) {
                std::string method = msg["method"];
                if (msg.contains("id")) { // It's a request
                    handle_request(msg["id"], method, msg.value("params", json::object()));
                } else { // It's a notification
                    handle_notification(method, msg.value("params", json::object()));
                }
            }
        } catch (json::parse_error& e) {
            log_message("JSON parse error: " + std::string(e.what()));
        }
    }

    void handle_request(const json& id, const std::string& method, const json& params) {
        if (method == "initialize") {
            on_initialize(id, params);
        } else if (method == "textDocument/completion") {
            on_completion(id, params);
        } else {
            // Unhandled request
        }
    }

    void handle_notification(const std::string& method, const json& params) {
        if (method == "initialized") {
            // Client is ready
        } else if (method == "textDocument/didOpen") {
            on_did_open(params);
        } else if (method == "textDocument/didChange") {
            on_did_change(params);
        }
    }

    void on_initialize(const json& id, const json& params) {
        json capabilities = {
            {"textDocumentSync", {
                {"openClose", true},
                {"change", 1} // 1 = Full sync
            }},
            {"completionProvider", {
                {"resolveProvider", false},
                {"triggerCharacters", {"@", "["}}
            }}
        };
        json response = {
            {"jsonrpc", "2.0"},
            {"id", id},
            {"result", {{"capabilities", capabilities}}}
        };
        send_response(response);
    }

    void on_did_open(const json& params) {
        std::string uri = params["textDocument"]["uri"];
        std::string content = params["textDocument"]["text"];
        validate_document(uri, content);
    }

    void on_did_change(const json& params) {
        std::string uri = params["textDocument"]["uri"];
        std::string content = params["contentChanges"][0]["text"];
        validate_document(uri, content);
    }

    void on_completion(const json& id, const json& params) {
        json result = {
            {"isIncomplete", false},
            {"items", json::array()}
        };

        std::string uri = params["textDocument"]["uri"];
        if (m_managers.count(uri)) {
            const auto& manager = m_managers.at(uri);
            const auto& macros = manager.interpreter.get_globals().get_all();
            for (const auto& pair : macros) {
                result["items"].push_back({
                    {"label", pair.first},
                    {"kind", 13} // 13 = Variable
                });
            }
        }

        json response = {
            {"jsonrpc", "2.0"},
            {"id", id},
            {"result", result}
        };
        send_response(response);
    }

    void validate_document(const std::string& uri, const std::string& content) {
        json diagnostics = json::array();
        try {
            YINI::YiniManager& manager = m_managers[uri];
            manager.load_from_string(content, uri);
        } catch (const YINI::YiniException& e) {
            json range = {
                {"start", {{"line", e.line() - 1}, {"character", e.column() - 1}}},
                {"end", {{"line", e.line() - 1}, {"character", e.column()}}}
            };
            diagnostics.push_back({
                {"range", range},
                {"severity", 1},
                {"source", "yini-lsp"},
                {"message", e.what()}
            });
        }

        json notification = {
            {"jsonrpc", "2.0"},
            {"method", "textDocument/publishDiagnostics"},
            {"params", {
                {"uri", uri},
                {"diagnostics", diagnostics}
            }}
        };
        send_notification(notification);
    }

    std::map<std::string, YINI::YiniManager> m_managers;
};

int main() {
    LspServer server;
    server.run();
    return 0;
}