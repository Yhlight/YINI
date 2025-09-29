#include <iostream>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void sendResponse(const json& response) {
    std::string responseStr = response.dump();
    std::cout << "Content-Length: " << responseStr.length() << "\r\n\r\n" << responseStr;
    std::cout.flush();
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

            // For now, just respond to the initialize request to establish connection
            if (receivedJson["method"] == "initialize") {
                json response = {
                    {"jsonrpc", "2.0"},
                    {"id", receivedJson["id"]},
                    {"result", {
                        {"capabilities", {}}
                    }}
                };
                sendResponse(response);
                std::cerr << "Sent initialize response." << std::endl;
            } else if (receivedJson["method"] == "initialized") {
                 // Client has been initialized, we can send a message
                json message = {
                    {"jsonrpc", "2.0"},
                    {"method", "window/showMessage"},
                    {"params", {
                        {"type", 3}, // 1: Error, 2: Warning, 3: Info, 4: Log
                        {"message", "YINI Language Server connected!"}
                    }}
                };
                sendResponse(message);
            } else if (receivedJson["method"] == "exit") {
                break;
            }

        } catch (const json::parse_error& e) {
            std::cerr << "JSON Parse Error: " << e.what() << std::endl;
        }
    }

    std::cerr << "YINI Language Server shutting down." << std::endl;
    return 0;
}