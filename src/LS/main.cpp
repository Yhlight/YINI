#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <map>
#include "nlohmann/json.hpp"
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "Resolver/Resolver.h"
#include "Validator/Validator.h"
#include "Ymeta/YmetaManager.h"

using json = nlohmann::json;

static std::map<std::string, std::string> open_documents;
static std::map<std::string, std::map<std::string, YINI::AST::SchemaRule>> parsed_schemas;

void send_json_response(const json& response)
{
    std::string response_str = response.dump();
    std::cout << "Content-Length: " << response_str.length() << "\r\n\r\n";
    std::cout << response_str << std::flush;
}

void parse_and_cache_schemas(const std::string& text)
{
    parsed_schemas.clear();
    try
    {
        YINI::Lexer lexer(text);
        auto tokens = lexer.scan_tokens();
        YINI::Parser parser(tokens);
        auto ast = parser.parse();
        for (const auto& stmt : ast)
        {
            if (auto* schema_stmt = dynamic_cast<YINI::AST::SchemaStmt*>(stmt.get()))
            {
                for (const auto& section : schema_stmt->sections)
                {
                    for (const auto& rule : section->rules)
                    {
                        parsed_schemas[section->name.lexeme][rule->key.lexeme] = rule->rule;
                    }
                }
            }
        }
    }
    catch (const std::exception& e) {}
}

void publish_diagnostics(const std::string& uri, const std::string& text)
{
    json diagnostics;
    try
    {
        YINI::Lexer lexer(text);
        auto tokens = lexer.scan_tokens();
        YINI::Parser parser(tokens);
        auto ast = parser.parse();
        YINI::YmetaManager ymeta_manager;
        YINI::Resolver resolver(ast, ymeta_manager);
        auto resolved_config = resolver.resolve();
        YINI::Validator validator(resolved_config, ast);
        validator.validate();
        diagnostics = json::array();
    }
    catch (const std::runtime_error& e)
    {
        diagnostics = json::array({
            {
                {"range", {
                    {"start", {{"line", 0}, {"character", 0}}},
                    {"end", {{"line", 1, 0}}}
                }},
                {"severity", 1},
                {"source", "yini-ls"},
                {"message", e.what()}
            }
        });
    }

    json params = {{"uri", uri}, {"diagnostics", diagnostics}};
    json notification = {
        {"jsonrpc", "2.0"},
        {"method", "textDocument/publishDiagnostics"},
        {"params", params}
    };
    send_json_response(notification);
}

json get_completion_items(const std::string& text, const std::string& trigger_char)
{
    json completion_items = json::array();
    try
    {
        YINI::Lexer lexer(text);
        auto tokens = lexer.scan_tokens();
        YINI::Parser parser(tokens);
        auto ast = parser.parse();

        if (trigger_char == "@")
        {
            for (const auto& stmt : ast)
            {
                if (auto* define_stmt = dynamic_cast<YINI::AST::DefineSectionStmt*>(stmt.get()))
                {
                    for (const auto& def : define_stmt->definitions)
                    {
                        json item = {
                            {"label", def->key.lexeme},
                            {"kind", 6}
                        };
                        completion_items.push_back(item);
                    }
                }
            }
        }
        else if (trigger_char == "{")
        {
            YINI::YmetaManager ymeta;
            YINI::Resolver resolver(ast, ymeta);
            auto config = resolver.resolve();
            for(auto const& [key, val] : config)
            {
                json item = {
                    {"label", key},
                    {"kind", 6}
                };
                completion_items.push_back(item);
            }
        }
    }
    catch (const std::exception& e) {}
    return completion_items;
}

std::string get_word_at_position(const std::string& text, int line, int character)
{
    int start = 0;
    int end = text.length();
    int current_line = 0;
    for(int i = 0; i < text.length(); ++i) {
        if (current_line == line) {
            if (i <= character && (text[i] == ' ' || text[i] == '\n' || text[i] == '=')) {
                start = i + 1;
            }
            if (i >= character && (text[i] == ' ' || text[i] == '\n' || text[i] == '=')) {
                end = i;
                break;
            }
        }
        if (text[i] == '\n') {
            current_line++;
        }
    }
    if (start >= end) return "";
    return text.substr(start, end - start);
}

void run_server_loop()
{
    while (true)
    {
        long long content_length = -1;
        std::string line;
        while (std::getline(std::cin, line) && !line.empty() && line != "\r")
        {
            if (line.rfind("Content-Length: ", 0) == 0)
            {
                content_length = std::stoll(line.substr(16));
            }
        }
        if (content_length == -1) continue;

        std::vector<char> buffer(content_length);
        std::cin.read(buffer.data(), content_length);
        json request = json::parse(buffer);

        if (request.contains("method"))
        {
            std::string method = request["method"];
            if (method == "initialize")
            {
                json response = {
                    {"jsonrpc", "2.0"},
                    {"id", request["id"]},
                    {"result", {
                        {"capabilities", {
                            {"textDocumentSync", 1},
                            {"hoverProvider", true},
                            {"completionProvider", {
                                {"triggerCharacters", {"@", "{"}}
                            }}
                        }}
                    }}
                };
                send_json_response(response);
            }
            else if (method == "textDocument/didOpen" || method == "textDocument/didChange")
            {
                std::string uri = request["params"]["textDocument"]["uri"];
                std::string text = (method == "textDocument/didOpen") ?
                                   request["params"]["textDocument"]["text"] :
                                   request["params"]["contentChanges"][0]["text"];
                open_documents[uri] = text;
                parse_and_cache_schemas(text);
                publish_diagnostics(uri, text);
            }
            else if (method == "textDocument/hover")
            {
                std::string uri = request["params"]["textDocument"]["uri"];
                int line = request["params"]["position"]["line"];
                int character = request["params"]["position"]["character"];
                std::string word = get_word_at_position(open_documents[uri], line, character);

                std::string markdown_string = "";
                for(const auto& section_pair : parsed_schemas)
                {
                    if (section_pair.second.count(word))
                    {
                        const auto& rule = section_pair.second.at(word);
                        markdown_string = "**" + word + "**\n\n";
                        markdown_string += "* Type: `" + rule.type + "`\n";
                        markdown_string += "* Required: `" + std::string(rule.requirement == YINI::AST::SchemaRule::Requirement::REQUIRED ? "true" : "false") + "`\n";
                        if(rule.default_value) markdown_string += "* Default: `" + *rule.default_value + "`\n";
                        if(rule.min) markdown_string += "* Min: `" + std::to_string(*rule.min) + "`\n";
                        if(rule.max) markdown_string += "* Max: `" + std::to_string(*rule.max) + "`\n";
                        break;
                    }
                }

                json result = nullptr;
                if (!markdown_string.empty()) {
                    result = {
                        {"contents", {
                            {"kind", "markdown"},
                            {"value", markdown_string}
                        }}
                    };
                }

                json response = {
                    {"jsonrpc", "2.0"},
                    {"id", request["id"]},
                    {"result", result}
                };
                send_json_response(response);
            }
            else if (method == "textDocument/completion")
            {
                std::string uri = request["params"]["textDocument"]["uri"];
                std::string trigger_char = request["params"]["context"]["triggerCharacter"];
                json items = get_completion_items(open_documents[uri], trigger_char);
                json response = {
                    {"jsonrpc", "2.0"},
                    {"id", request["id"]},
                    {"result", items}
                };
                send_json_response(response);
            }
            else if (method == "shutdown")
            {
                json response = {{"jsonrpc", "2.0"}, {"id", request["id"]}, {"result", nullptr}};
                send_json_response(response);
            }
            else if (method == "exit")
            {
                return;
            }
        }
    }
}

int main(int argc, char* argv[])
{
    std::cerr << "YINI Language Server starting." << std::endl;
    run_server_loop();
    std::cerr << "YINI Language Server shutting down." << std::endl;
    return 0;
}
