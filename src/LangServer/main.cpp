#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include "Parser/parser.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// --- Document State ---
struct DocumentState {
    std::string text;
    Config config;
    std::map<std::string, Parser::MacroDefinition> macroMap;
};
std::map<std::string, DocumentState> open_documents;


// --- LSP Communication Helpers ---
void send_response(const json& id, const json& result) {
    json response = {
        {"jsonrpc", "2.0"},
        {"id", id},
        {"result", result}
    };
    std::string content = response.dump();
    std::cout << "Content-Length: " << content.length() << "\r\n\r\n" << content;
    std::cout.flush();
}

void send_notification(const std::string& method, const json& params) {
    json notification = {
        {"jsonrpc", "2.0"},
        {"method", method},
        {"params", params}
    };
    std::string content = notification.dump();
    std::cout << "Content-Length: " << content.length() << "\r\n\r\n" << content;
    std::cout.flush();
}

// --- LSP Handlers ---
void validate_document(const std::string& uri, const std::string& text) {
    std::vector<json> diagnostics;
    try {
        // We need a way to get the file path from the URI
        std::string file_path = uri;
        if (file_path.rfind("file://", 0) == 0) {
            file_path = file_path.substr(7);
        }
        std::filesystem::path p(file_path);
        std::string base_dir = p.parent_path().string();

        Parser parser;
        Config config = parser.parse(text, base_dir); // Parse text content directly
        parser.validate(config);
        open_documents[uri] = {text, std::move(config), parser.getMacroMap()};
    } catch (const ParsingException& e) {
        size_t line = e.getLine() > 0 ? e.getLine() - 1 : 0;
        size_t col = e.getColumn() > 0 ? e.getColumn() - 1 : 0;
        diagnostics.push_back({
            {"range", {
                {"start", {{"line", line}, {"character", col}}},
                {"end", {{"line", line}, {"character", col + 1}}}
            }},
            {"severity", 1}, // Error
            {"source", "yini"},
            {"message", e.what()}
        });
        open_documents.erase(uri);
        open_documents[uri].text = text;
    } catch (const std::exception& e) {
        diagnostics.push_back({
            {"range", {
                {"start", {{"line", 0}, {"character", 0}}},
                {"end", {{"line", 0}, {"character", 1}}}
            }},
            {"severity", 1}, // Error
            {"source", "yini"},
            {"message", e.what()}
        });
        open_documents.erase(uri);
        open_documents[uri].text = text;
    }

    send_notification("textDocument/publishDiagnostics", {
        {"uri", uri},
        {"diagnostics", diagnostics}
    });
}

// --- Main Loop ---
int main() {
    send_notification("window/logMessage", {{"type", 3}, {"message", "YINI Language Server started."}});

    while (true) {
        std::string content_length_header;
        if (!std::getline(std::cin, content_length_header) || content_length_header.empty()) {
            break;
        }

        if (content_length_header.rfind("Content-Length: ", 0) != 0) {
            continue;
        }
        int content_length = 0;
        try {
            content_length = std::stoi(content_length_header.substr(16));
        } catch (...) {
            continue;
        }

        std::string dummy;
        std::getline(std::cin, dummy);
        std::vector<char> buffer(content_length);
        std::cin.read(buffer.data(), content_length);
        std::string content_str(buffer.begin(), buffer.end());

        try {
            json request = json::parse(content_str);

            if (request.contains("method")) {
                std::string method = request["method"];

                if (!request.contains("id")) { // Notification
                    if (method == "textDocument/didOpen") {
                        std::string uri = request["params"]["textDocument"]["uri"];
                        std::string text = request["params"]["textDocument"]["text"];
                        validate_document(uri, text);
                    } else if (method == "textDocument/didChange") {
                        std::string uri = request["params"]["textDocument"]["uri"];
                        std::string text = request["params"]["contentChanges"][0]["text"];
                        validate_document(uri, text);
                    } else if (method == "exit") {
                        break;
                    }
                } else { // Request
                    json id = request["id"];
                    if (method == "initialize") {
                        json capabilities = {
                            {"textDocumentSync", 1}, // Full sync
                            {"hoverProvider", true},
                            {"completionProvider", {
                                {"triggerCharacters", {"[", ".", "@"}}
                            }},
                            {"definitionProvider", true}
                        };
                        send_response(id, {{"capabilities", capabilities}});
                    } else if (method == "textDocument/definition") {
                        std::string uri = request["params"]["textDocument"]["uri"];
                        if (open_documents.count(uri)) {
                            const auto& state = open_documents.at(uri);
                            int line = request["params"]["position"]["line"];
                            int character = request["params"]["position"]["character"];

                            Lexer lexer(state.text);
                            std::vector<Token> tokens = lexer.allTokens();

                            Token* found_token = nullptr;
                            for (auto& token : tokens) {
                                if (token.type == TokenType::Identifier &&
                                    token.line == static_cast<size_t>(line + 1) &&
                                    token.column <= static_cast<size_t>(character + 1) &&
                                    static_cast<size_t>(character + 1) < token.column + token.value.length()) {
                                    found_token = &token;
                                    break;
                                }
                            }

                            if (found_token && state.macroMap.count(found_token->value)) {
                                const auto& def = state.macroMap.at(found_token->value);
                                json location = {
                                    {"uri", uri},
                                    {"range", {
                                        {"start", {{"line", def.line - 1}, {"character", def.column - 1}}},
                                        {"end", {{"line", def.line - 1}, {"character", def.column - 1 + found_token->value.length()}}}
                                    }}
                                };
                                send_response(id, location);
                            } else {
                                send_response(id, nullptr);
                            }
                        } else {
                            send_response(id, nullptr);
                        }
                    }
                    else if (method == "textDocument/completion") {
                        std::string uri = request["params"]["textDocument"]["uri"];
                        json position = request["params"]["position"];
                        std::vector<json> completions;

                        if (open_documents.count(uri)) {
                            const auto& state = open_documents.at(uri);
                            std::string line_text;
                            std::stringstream text_stream(state.text);
                            for(int i = 0; i <= position["line"].get<int>(); ++i) {
                                std::getline(text_stream, line_text);
                            }

                            char trigger_char = 0;
                            if (position["character"].get<int>() > 0) {
                                trigger_char = line_text[position["character"].get<int>() - 1];
                            }

                            if (trigger_char == '[') {
                                for(auto const& [key, val] : state.config) {
                                    completions.push_back({
                                        {"label", key},
                                        {"kind", 14} // Module
                                    });
                                }
                            } else if (trigger_char == '.') {
                                // Simple check for cross-section reference completion
                                size_t at_brace_pos = line_text.rfind("@{", position["character"].get<int>());
                                if (at_brace_pos != std::string::npos) {
                                    std::string section_name = line_text.substr(at_brace_pos + 2, position["character"].get<int>() - at_brace_pos - 3);
                                    if (state.config.count(section_name)) {
                                        const auto& section = state.config.at(section_name);
                                        for(auto const& [key, val] : section) {
                                            completions.push_back({
                                                {"label", key},
                                                {"kind", 5} // Field
                                            });
                                        }
                                    }
                                }
                            }
                        }
                        send_response(id, completions);
                    } else if (method == "textDocument/hover") {
                        std::string uri = request["params"]["textDocument"]["uri"];
                        if (open_documents.count(uri)) {
                            const auto& state = open_documents.at(uri);
                            int line = request["params"]["position"]["line"];
                            int character = request["params"]["position"]["character"];

                            Lexer lexer(state.text);
                            std::vector<Token> tokens = lexer.allTokens();

                            Token* found_token = nullptr;
                            for (auto& token : tokens) {
                                if (token.line == static_cast<size_t>(line + 1) &&
                                    token.column <= static_cast<size_t>(character + 1) &&
                                    static_cast<size_t>(character + 1) < token.column + token.value.length()) {
                                    found_token = &token;
                                    break;
                                }
                            }

                            if (found_token) {
                                std::string hover_content = "Value: `" + found_token->value + "`";
                                json contents = {
                                    {"kind", "markdown"},
                                    {"value", hover_content}
                                };
                                send_response(id, {{"contents", contents}});
                            } else {
                                send_response(id, nullptr);
                            }
                        } else {
                            send_response(id, nullptr);
                        }
                    } else {
                        send_response(id, nullptr); // Method not found
                    }
                }
            }
        } catch (const json::parse_error&) {
            // Ignore if not a valid JSON request
        }
    }

    return 0;
}