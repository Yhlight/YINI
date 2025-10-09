#include <iostream>
#include <string>
#include <vector>
#include "Parser/parser.h"
#include <nlohmann/json.hpp>

void log_message(const std::string& message) {
    std::cerr << "[LangServer] " << message << std::endl;
}

void send_diagnostic(const std::string& uri, const std::string& message) {
    nlohmann::json diagnostic = {
        {"jsonrpc", "2.0"},
        {"method", "textDocument/publishDiagnostics"},
        {"params", {
            {"uri", uri},
            {"diagnostics", {
                {
                    {"range", {
                        {"start", {{"line", 0}, {"character", 0}}},
                        {"end", {{"line", 0}, {"character", 1}}}
                    }},
                    {"severity", 1}, // 1 for Error
                    {"source", "yini"},
                    {"message", message}
                }
            }}
        }}
    };
    std::cout << "Content-Length: " << diagnostic.dump().length() << "\r\n\r\n" << diagnostic.dump();
    std::cout.flush();
}

void clear_diagnostics(const std::string& uri) {
     nlohmann::json diagnostic = {
        {"jsonrpc", "2.0"},
        {"method", "textDocument/publishDiagnostics"},
        {"params", {
            {"uri", uri},
            {"diagnostics", nlohmann::json::array()}
        }}
    };
    std::cout << "Content-Length: " << diagnostic.dump().length() << "\r\n\r\n" << diagnostic.dump();
    std::cout.flush();
}


int main() {
    log_message("Language server started.");

    std::string line;
    while (std::getline(std::cin, line)) {
        // A very basic protocol: each line is a file path to check.
        // A real LSP would have a much more complex JSON-RPC protocol.
        std::string file_path = line;
        log_message("Received file path: " + file_path);

        try {
            Parser parser;
            Config config = parser.parseFile(file_path);
            parser.validate(config);
            log_message("Validation successful for: " + file_path);
            clear_diagnostics("file://" + file_path);
        } catch (const std::exception& e) {
            log_message("Validation failed for: " + file_path + " | Error: " + e.what());
            send_diagnostic("file://" + file_path, e.what());
        }
    }

    log_message("Language server shutting down.");
    return 0;
}