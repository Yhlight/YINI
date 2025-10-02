#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <cstdio>
#include <memory>
#include <array>
#include <thread>
#include "nlohmann/json.hpp"

// Macros to convert the preprocessor definition to a string
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

using json = nlohmann::json;

// Helper to send a raw message string to the LSP server and get the response
std::string send_raw_message(const std::string& message) {
    std::string lsp_path = TOSTRING(YINI_LSP_PATH);
    // Add a small sleep to give the LSP server time to process and respond
    // before the pipe is closed. Use printf to avoid echo's escape sequence interpretation.
    std::string cmd = "(printf '%s' '" + message + "'; sleep 0.2) | " + lsp_path;

    std::array<char, 512> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

// Helper to construct a JSON-RPC request and return the parsed JSON response
json send_request(int& id, const std::string& method, const json& params) {
    id++;
    json request = {
        {"jsonrpc", "2.0"},
        {"id", id},
        {"method", method},
        {"params", params}
    };
    std::string request_str = request.dump();
    std::string message = "Content-Length: " + std::to_string(request_str.length()) + "\r\n\r\n" + request_str;

    std::string response_str = send_raw_message(message);

    size_t header_end = response_str.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        throw std::runtime_error("Malformed LSP response: " + response_str);
    }
    std::string json_part = response_str.substr(header_end + 4);
    return json::parse(json_part);
}

TEST(LspTest, Initialize) {
    int request_id = 0;
    json params = {
        {"processId", 1234},
        {"rootUri", "file:///tmp"},
        {"capabilities", {}}
    };
    json response = send_request(request_id, "initialize", params);

    EXPECT_EQ(response["id"], 1);
    EXPECT_TRUE(response["result"]["capabilities"].is_object());
    EXPECT_TRUE(response["result"]["capabilities"]["textDocumentSync"]["openClose"]);
    EXPECT_EQ(response["result"]["capabilities"]["textDocumentSync"]["change"], 1);
    EXPECT_TRUE(response["result"]["capabilities"]["completionProvider"].is_object());
    auto triggerChars = response["result"]["capabilities"]["completionProvider"]["triggerCharacters"];
    ASSERT_EQ(triggerChars.size(), 2);
    EXPECT_EQ(triggerChars[0], "@");
    EXPECT_EQ(triggerChars[1], "[");
}

TEST(LspTest, PublishDiagnostics) {
    int request_id = 0;
    std::string content = "[Section\nkey = value"; // Invalid syntax
    json didOpenParams = {
        {"textDocument", {
            {"uri", "file:///test.yini"},
            {"languageId", "yini"},
            {"version", 1},
            {"text", content}
        }}
    };
    json notification = {
        {"jsonrpc", "2.0"},
        {"method", "textDocument/didOpen"},
        {"params", didOpenParams}
    };
    std::string notification_str = notification.dump();
    std::string message = "Content-Length: " + std::to_string(notification_str.length()) + "\r\n\r\n" + notification_str;

    std::string response_str = send_raw_message(message);

    // We expect a textDocument/publishDiagnostics notification
    size_t header_end = response_str.find("\r\n\r\n");
    ASSERT_NE(header_end, std::string::npos);
    std::string json_part = response_str.substr(header_end + 4);
    json diagnostics_notification = json::parse(json_part);

    EXPECT_EQ(diagnostics_notification["method"], "textDocument/publishDiagnostics");
    const auto& diagnostics = diagnostics_notification["params"]["diagnostics"];
    ASSERT_EQ(diagnostics.size(), 1);
    EXPECT_EQ(diagnostics[0]["severity"], 1); // Error
    EXPECT_NE(diagnostics[0]["message"].get<std::string>().find("Expect ']' after section name."), std::string::npos);
}

// Helper to parse multiple JSON responses from a single string
std::vector<json> parse_responses(const std::string& raw_response) {
    std::vector<json> responses;
    size_t pos = 0;
    while(pos < raw_response.length()) {
        size_t cl_pos = raw_response.find("Content-Length: ", pos);
        if (cl_pos == std::string::npos) break;

        size_t header_end_pos = raw_response.find("\r\n\r\n", cl_pos);
        if (header_end_pos == std::string::npos) break;

        std::string cl_str = raw_response.substr(cl_pos + 16, header_end_pos - (cl_pos + 16));
        long content_length = 0;
        try {
            content_length = std::stol(cl_str);
        } catch (...) { break; }

        if (raw_response.length() < header_end_pos + 4 + content_length) break;

        std::string json_part = raw_response.substr(header_end_pos + 4, content_length);
        try {
            responses.push_back(json::parse(json_part));
        } catch(const json::parse_error& e) {
            // Can happen if the pipe is closed mid-write, just ignore malformed parts
        }

        pos = header_end_pos + 4 + content_length;
    }
    return responses;
}

TEST(LspTest, Completion) {
    int request_id = 0;
    std::string content = "[#define]\nmy_var = 123\n\n[MySection]\nkey = @";

    // Construct didOpen notification
    json didOpenParams = {
        {"textDocument", {
            {"uri", "file:///test.yini"},
            {"languageId", "yini"},
            {"version", 1},
            {"text", content}
        }}
    };
    json didOpenNotification = {
        {"jsonrpc", "2.0"},
        {"method", "textDocument/didOpen"},
        {"params", didOpenParams}
    };
    std::string didOpenStr = didOpenNotification.dump();
    std::string didOpenMessage = "Content-Length: " + std::to_string(didOpenStr.length()) + "\r\n\r\n" + didOpenStr;

    // Construct completion request
    request_id++;
    json completionParams = {
        {"textDocument", {{"uri", "file:///test.yini"}}},
        {"position", {{"line", 4}, {"character", 7}}}
    };
    json completionRequest = {
        {"jsonrpc", "2.0"},
        {"id", request_id},
        {"method", "textDocument/completion"},
        {"params", completionParams}
    };
    std::string completionRequestStr = completionRequest.dump();
    std::string completionRequestMessage = "Content-Length: " + std::to_string(completionRequestStr.length()) + "\r\n\r\n" + completionRequestStr;

    // Send both messages in one go
    std::string raw_response = send_raw_message(didOpenMessage + completionRequestMessage);
    std::vector<json> responses = parse_responses(raw_response);

    // Find the completion response
    json completionResponse;
    bool found = false;
    for(const auto& resp : responses) {
        if(resp.contains("id") && resp["id"] == request_id) {
            completionResponse = resp;
            found = true;
            break;
        }
    }
    ASSERT_TRUE(found) << "Did not find completion response in server output.";

    const auto& items = completionResponse["result"]["items"];
    ASSERT_EQ(items.size(), 1);
    EXPECT_EQ(items[0]["label"], "my_var");
}