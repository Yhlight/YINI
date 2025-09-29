#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>
#include "YINI/Parser.hpp"
#include "YINI/Lexer.hpp"

using json = nlohmann::json;

// A map to store the parsed document state for each open file
std::map<std::string, YINI::YiniDocument> documentStates;
// A map to store the raw text content of open documents
std::map<std::string, std::string> openDocuments;

void sendResponse(const json &response)
{
    std::string responseStr = response.dump();
    std::cout << "Content-Length: " << responseStr.length() << "\r\n\r\n" << responseStr;
    std::cout.flush();
}

void parseAndStoreDocument(const std::string& uri, const std::string& text)
{
    openDocuments[uri] = text;
    try
    {
        YINI::YiniDocument doc;
        YINI::Parser parser(text, doc);
        parser.parse();
        documentStates[uri] = std::move(doc);
        std::cerr << "Parsed and stored document: " << uri << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error parsing document " << uri << ": " << e.what() << std::endl;
    }
}

int main()
{
    std::cerr << "YINI Language Server starting..." << std::endl;

    while (true)
    {
        long long contentLength = -1;
        std::string line;
        while (std::getline(std::cin, line) && !line.empty() && line != "\r")
        {
            if (line.rfind("Content-Length: ", 0) == 0)
            {
                contentLength = std::stoll(line.substr(16));
            }
        }

        if (contentLength == -1)
        {
            continue;
        }

        std::vector<char> content(contentLength);
        std::cin.read(content.data(), contentLength);
        std::string contentStr(content.begin(), content.end());

        std::cerr << "Received message: " << contentStr << std::endl;

        try
        {
            json receivedJson = json::parse(contentStr);

            if (receivedJson.contains("method"))
            {
                std::string method = receivedJson["method"];
                if (method == "initialize")
                {
                    json response = {
                        {"jsonrpc", "2.0"},
                        {"id", receivedJson["id"]},
                        {"result", {
                            {"capabilities", {
                                {"textDocumentSync", 1}, // Full sync
                                {"definitionProvider", true}
                            }}
                        }}
                    };
                    sendResponse(response);
                    std::cerr << "Sent initialize response." << std::endl;
                }
                else if (method == "initialized")
                {
                    json message = {
                        {"jsonrpc", "2.0"},
                        {"method", "window/showMessage"},
                        {"params", {
                            {"type", 3},
                            {"message", "YINI Language Server connected!"}
                        }}
                    };
                    sendResponse(message);
                }
                else if (method == "textDocument/didOpen")
                {
                    std::string uri = receivedJson["params"]["textDocument"]["uri"];
                    std::string text = receivedJson["params"]["textDocument"]["text"];
                    parseAndStoreDocument(uri, text);
                }
                else if (method == "textDocument/didChange")
                {
                    std::string uri = receivedJson["params"]["textDocument"]["uri"];
                    std::string text = receivedJson["params"]["contentChanges"][0]["text"];
                    parseAndStoreDocument(uri, text);
                }
                else if (method == "textDocument/didClose")
                {
                    std::string uri = receivedJson["params"]["textDocument"]["uri"];
                    documentStates.erase(uri);
                    openDocuments.erase(uri);
                    std::cerr << "Closed document: " << uri << std::endl;
                }
                else if (method == "textDocument/definition")
                {
                    std::string uri = receivedJson["params"]["textDocument"]["uri"];
                    int line = receivedJson["params"]["position"]["line"];
                    int character = receivedJson["params"]["position"]["character"];

                    json result = nullptr;
                    if (documentStates.count(uri) && openDocuments.count(uri))
                    {
                        YINI::YiniDocument& doc = documentStates.at(uri);
                        std::string& docContent = openDocuments.at(uri);

                        std::string macro_key;
                        YINI::Lexer lexer(docContent);
                        YINI::Token token;
                        while((token = lexer.getNextToken()).type != YINI::TokenType::Eof)
                        {
                            if (token.type == YINI::TokenType::At && token.line - 1 == line)
                            {
                                 YINI::Token next_token = lexer.getNextToken();
                                 if (next_token.type == YINI::TokenType::Identifier && character >= next_token.column -1 && character < (next_token.column -1 + next_token.value.length()))
                                 {
                                     macro_key = next_token.value;
                                     break;
                                 }
                            }
                        }

                        if (!macro_key.empty())
                        {
                            const YINI::MacroDefinition* def = doc.getMacroDefinition(macro_key);
                            if (def)
                            {
                                result = json::array({
                                    {
                                        {"uri", uri},
                                        {"range", {
                                            {"start", {{"line", def->location.line - 1}, {"character", def->location.column - 1}}},
                                            {"end", {{"line", def->location.line - 1}, {"character", def->location.column - 1 + (int)macro_key.length()}}}
                                        }}
                                    }
                                });
                            }
                        }
                    }

                    json response = {
                        {"jsonrpc", "2.0"},
                        {"id", receivedJson["id"]},
                        {"result", result}
                    };
                    sendResponse(response);
                }
                else if (method == "exit")
                {
                    break;
                }
            }
        }
        catch (const json::parse_error &e)
        {
            std::cerr << "JSON Parse Error: " << e.what() << std::endl;
        }
        catch (const std::exception& e)
        {
             std::cerr << "Error: " << e.what() << std::endl;
        }
    }

    std::cerr << "YINI Language Server shutting down." << std::endl;
    return 0;
}