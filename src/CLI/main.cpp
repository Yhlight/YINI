#include "lz4.h"
#include <cmath>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

#include "Lexer/Lexer.h"
#include "Loader/YbinFormat.h"
#include "Loader/YbinSerialization.h"
#include "Parser/Parser.h"
#include "Resolver/Resolver.h"
#include "Resolver/SemanticInfoVisitor.h"
#include "Utils/Endian.h"
#include "Validator/Validator.h"
#include "YiniTypes.h"
#include "Ymeta/YmetaManager.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

// A simple document manager to store the content of open files
static std::map<std::string, std::string> document_manager;
static std::map<std::string, json> semantic_info_cache;

void log_message(const std::string &msg)
{
    std::cerr << "YINI LS: " << msg << std::endl;
}

void send_json_rpc(const json &msg)
{
    std::string dumped_msg = msg.dump();
    std::cout << "Content-Length: " << dumped_msg.length() << "\r\n\r\n" << dumped_msg << std::flush;
    log_message("Sent: " + dumped_msg);
}

// Function to parse a document and update the semantic info cache
void update_document_info(const std::string &uri, const std::string &text)
{
    document_manager[uri] = text;
    try
    {
        YINI::Lexer lexer(text);
        auto tokens = lexer.scan_tokens();
        YINI::Parser parser(tokens);
        auto ast = parser.parse();

        YINI::SemanticInfoVisitor visitor(text, uri);
        for (const auto &stmt : ast)
        {
            if (stmt)
                stmt->accept(&visitor);
        }
        semantic_info_cache[uri] = visitor.get_info();
    }
    catch (const std::exception &e)
    {
        // Handle parse errors, maybe send diagnostics
        log_message("Error parsing document " + uri + ": " + e.what());
        semantic_info_cache.erase(uri);
    }
}

void run_language_server()
{
    log_message("Language server started.");

    while (std::cin.good())
    {
        std::string line;
        long length = 0;
        while (std::getline(std::cin, line) && !line.empty() && line != "\r")
        {
            if (line.rfind("Content-Length: ", 0) == 0)
            {
                length = std::stol(line.substr(16));
            }
        }

        if (length == 0)
        {
            continue;
        }

        std::string content(length, '\0');
        std::cin.read(&content[0], length);

        log_message("Received: " + content);

        json request = json::parse(content, nullptr, false);
        if (request.is_discarded())
        {
            log_message("Failed to parse JSON request.");
            continue;
        }

        if (request.contains("method"))
        {
            std::string method = request["method"];
            if (method == "initialize")
            {
                json response;
                response["id"] = request["id"];
                response["jsonrpc"] = "2.0";
                response["result"]["capabilities"]["semanticTokensProvider"]["legend"] = {
                    {"tokenTypes",
                     {"string", "number", "operator", "macro", "namespace", "property", "variable", "class"}},
                    {"tokenModifiers", {"readonly"}}};
                response["result"]["capabilities"]["semanticTokensProvider"]["full"] = true;
                response["result"]["capabilities"]["hoverProvider"] = true;
                response["result"]["capabilities"]["definitionProvider"] = true;
                response["result"]["capabilities"]["textDocumentSync"] = 1; // Full sync
                send_json_rpc(response);
            }
            else if (method == "textDocument/didOpen")
            {
                std::string uri = request["params"]["textDocument"]["uri"];
                std::string text = request["params"]["textDocument"]["text"];
                update_document_info(uri, text);
            }
            else if (method == "textDocument/didChange")
            {
                std::string uri = request["params"]["textDocument"]["uri"];
                std::string text = request["params"]["contentChanges"][0]["text"];
                update_document_info(uri, text);
            }
            else if (method == "textDocument/semanticTokens/full")
            {
                std::string uri = request["params"]["textDocument"]["uri"];
                json response;
                response["id"] = request["id"];
                response["jsonrpc"] = "2.0";
                if (semantic_info_cache.count(uri))
                {
                    // This is a simplified example. A real LSP would convert the stored
                    // token info into the required integer array format.
                    // For now, we return nothing to avoid client errors with wrong format.
                    response["result"]["data"] = json::array();
                }
                else
                {
                    response["result"]["data"] = json::array();
                }
                send_json_rpc(response);
            }
            else if (method == "textDocument/hover")
            {
                std::string uri = request["params"]["textDocument"]["uri"];
                int line = request["params"]["position"]["line"];
                int character = request["params"]["position"]["character"];

                json response;
                response["id"] = request["id"];
                response["jsonrpc"] = "2.0";
                response["result"] = nullptr;

                if (semantic_info_cache.count(uri))
                {
                    const auto &info = semantic_info_cache[uri];
                    for (const auto &token : info["tokens"])
                    {
                        if (token["line"] == line && character >= token["startChar"].get<int>() &&
                            character < (token["startChar"].get<int>() + token["length"].get<int>()))
                        {
                            response["result"]["contents"] = {
                                {"kind", "markdown"},
                                {"value", "Type: `" + token["tokenType"].get<std::string>() + "`"}};
                            break;
                        }
                    }
                }
                send_json_rpc(response);
            }
            else if (method == "textDocument/definition")
            {
                std::string uri = request["params"]["textDocument"]["uri"];
                int line = request["params"]["position"]["line"];
                int character = request["params"]["position"]["character"];

                json response;
                response["id"] = request["id"];
                response["jsonrpc"] = "2.0";
                response["result"] = nullptr;

                if (semantic_info_cache.count(uri))
                {
                    const auto &info = semantic_info_cache[uri];
                    std::string target_symbol_name;

                    // Find the token at the cursor position
                    for (const auto &token : info["tokens"])
                    {
                        if (token["line"] == line && character >= token["startChar"].get<int>() &&
                            character < (token["startChar"].get<int>() + token["length"].get<int>()))
                        {
                            if (token["tokenType"] == "macro")
                            {
                                // Extract the macro name from the document
                                const std::string &doc = document_manager[uri];
                                std::stringstream ss(doc);
                                std::string doc_line;
                                for (int i = 0; i <= line; ++i)
                                    std::getline(ss, doc_line);

                                std::string lexeme =
                                    doc_line.substr(token["startChar"].get<int>(), token["length"].get<int>());
                                if (lexeme.rfind("@", 0) == 0)
                                {
                                    target_symbol_name = lexeme.substr(1);
                                }
                            }
                            break;
                        }
                    }

                    if (!target_symbol_name.empty())
                    {
                        for (const auto &symbol : info["symbols"])
                        {
                            if (symbol["name"] == target_symbol_name)
                            {
                                response["result"] = symbol["location"];
                                break;
                            }
                        }
                    }
                }
                send_json_rpc(response);
            }
            else if (method == "shutdown")
            {
                json response;
                response["id"] = request["id"];
                response["jsonrpc"] = "2.0";
                response["result"] = nullptr;
                send_json_rpc(response);
            }
            else if (method == "exit")
            {
                return;
            }
        }
    }
}

// Helper class to build the string table
class StringTableBuilder
{
  public:
    uint32_t add(const std::string &s)
    {
        if (m_offsets.count(s))
        {
            return m_offsets[s];
        }
        uint32_t offset = m_blob.size();
        m_blob.append(s).push_back('\0');
        m_offsets[s] = offset;
        return offset;
    }

    const std::string &get_blob() const
    {
        return m_blob;
    }

  private:
    std::map<std::string, uint32_t> m_offsets;
    std::string m_blob;
};

// Returns the ybin type for a given variant, specifically for arrays
static YINI::Ybin::ValueType get_array_value_type(const YINI::YiniArray &arr)
{
    if (arr.empty())
        return YINI::Ybin::ValueType::Null;

    const auto &first_type = arr.front();
    if (std::holds_alternative<int64_t>(first_type))
        return YINI::Ybin::ValueType::ArrayInt;
    if (std::holds_alternative<double>(first_type))
        return YINI::Ybin::ValueType::ArrayDouble;
    if (std::holds_alternative<bool>(first_type))
        return YINI::Ybin::ValueType::ArrayBool;
    if (std::holds_alternative<std::string>(first_type))
        return YINI::Ybin::ValueType::ArrayString;

    return YINI::Ybin::ValueType::Null; // Unsupported array type
}

static void run_cook(const std::string &output_path, const std::vector<std::string> &input_paths)
{
    std::map<std::string, YINI::YiniVariant> combined_config;

    // 1. Parse and resolve all input files, merging them
    for (const auto &path : input_paths)
    {
        std::ifstream file(path);
        if (!file.is_open())
        {
            throw std::runtime_error("Could not open input file: " + path);
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string source = buffer.str();

        YINI::Lexer lexer(source);
        auto tokens = lexer.scan_tokens();
        YINI::Parser parser(tokens);
        auto ast = parser.parse();

        YINI::YmetaManager ymeta_manager;
        YINI::Resolver resolver(ast, ymeta_manager);
        auto resolved_config = resolver.resolve();

        combined_config.insert(resolved_config.begin(), resolved_config.end());
    }

    // 2. Build the data structures for the ybin file
    StringTableBuilder string_table;
    std::vector<char> data_table_buffer;
    YINI::Ybin::BufferWriter data_table(data_table_buffer);
    std::vector<YINI::Ybin::HashTableEntry> entries;

    for (const auto &pair : combined_config)
    {
        const std::string &key = pair.first;
        const YINI::YiniVariant &value = pair.second;

        YINI::Ybin::HashTableEntry entry;
        entry.key_hash = std::hash<std::string>{}(key);
        entry.key_offset = string_table.add(key);
        entry.next_entry_index = 0xFFFFFFFF;

        std::visit(
            [&](auto &&arg)
            {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, std::monostate>)
                {
                    entry.value_type = YINI::Ybin::ValueType::Null;
                    entry.value_offset = 0;
                }
                else if constexpr (std::is_same_v<T, int64_t>)
                {
                    entry.value_type = YINI::Ybin::ValueType::Int64;
                    entry.value_offset = data_table.size();
                    data_table.write_u64_le(arg);
                }
                else if constexpr (std::is_same_v<T, double>)
                {
                    entry.value_type = YINI::Ybin::ValueType::Double;
                    entry.value_offset = data_table.size();
                    data_table.write_double_le(arg);
                }
                else if constexpr (std::is_same_v<T, bool>)
                {
                    entry.value_type = YINI::Ybin::ValueType::Bool;
                    entry.value_offset = arg ? 1 : 0;
                }
                else if constexpr (std::is_same_v<T, std::string>)
                {
                    entry.value_type = YINI::Ybin::ValueType::String;
                    entry.value_offset = string_table.add(arg);
                }
                else if constexpr (std::is_same_v<T, YINI::ResolvedColor>)
                {
                    entry.value_type = YINI::Ybin::ValueType::Color;
                    YINI::Ybin::ColorData cdata{arg.r, arg.g, arg.b};
                    entry.value_offset = data_table.size();
                    data_table.write(cdata);
                }
                else if constexpr (std::is_same_v<T, std::unique_ptr<YINI::YiniArray>>)
                {
                    const auto &arr = *arg;
                    entry.value_type = get_array_value_type(arr);
                    entry.value_offset = data_table.size();
                    data_table.write_u32_le((uint32_t)arr.size());

                    if (entry.value_type == YINI::Ybin::ValueType::ArrayInt)
                    {
                        for (const auto &item : arr)
                            data_table.write_u64_le(std::get<int64_t>(item));
                    }
                    else if (entry.value_type == YINI::Ybin::ValueType::ArrayDouble)
                    {
                        for (const auto &item : arr)
                            data_table.write_double_le(std::get<double>(item));
                    }
                    else if (entry.value_type == YINI::Ybin::ValueType::ArrayBool)
                    {
                        for (const auto &item : arr)
                            data_table.write(std::get<bool>(item));
                    }
                    else if (entry.value_type == YINI::Ybin::ValueType::ArrayString)
                    {
                        for (const auto &item : arr)
                        {
                            uint32_t str_offset = string_table.add(std::get<std::string>(item));
                            data_table.write_u32_le(str_offset);
                        }
                    }
                }
            },
            value);

        if (entry.value_type != YINI::Ybin::ValueType::Null)
        {
            entries.push_back(entry);
        }
    }

    // 3. Build the hash table buckets
    size_t bucket_count = entries.size() * 1.5;
    if (bucket_count == 0)
        bucket_count = 1;
    std::vector<uint32_t> buckets(bucket_count, 0xFFFFFFFF);

    for (uint32_t i = 0; i < entries.size(); ++i)
    {
        uint64_t key_hash_le = YINI::Utils::htole64(entries[i].key_hash);
        uint32_t bucket_index = key_hash_le % bucket_count;
        if (buckets[bucket_index] == 0xFFFFFFFF)
        {
            buckets[bucket_index] = YINI::Utils::htole32(i);
        }
        else
        {
            entries[i].next_entry_index = buckets[bucket_index];
            buckets[bucket_index] = YINI::Utils::htole32(i);
        }
    }

    // 4. Compress data and string tables
    const auto &uncompressed_data = data_table_buffer;
    int data_max_size = LZ4_compressBound(uncompressed_data.size());
    std::vector<char> compressed_data(data_max_size);
    int data_compressed_size =
        LZ4_compress_default(uncompressed_data.data(), compressed_data.data(), uncompressed_data.size(), data_max_size);
    compressed_data.resize(data_compressed_size);

    const auto &uncompressed_strings = string_table.get_blob();
    int string_max_size = LZ4_compressBound(uncompressed_strings.size());
    std::vector<char> compressed_strings(string_max_size);
    int string_compressed_size = LZ4_compress_default(uncompressed_strings.data(), compressed_strings.data(),
                                                      uncompressed_strings.size(), string_max_size);
    compressed_strings.resize(string_compressed_size);

    // 5. Write everything to the file
    std::ofstream out(output_path, std::ios::binary);
    if (!out)
    {
        throw std::runtime_error("Could not open output file for writing: " + output_path);
    }

    YINI::Ybin::FileHeader header;
    header.magic = YINI::Utils::htole32(YINI::Ybin::YBIN_MAGIC);
    header.version = YINI::Utils::htole32(2);
    header.entries_count = YINI::Utils::htole32(entries.size());
    header.hash_table_size = YINI::Utils::htole32(buckets.size());
    header.data_table_uncompressed_size = YINI::Utils::htole32(uncompressed_data.size());
    header.data_table_compressed_size = YINI::Utils::htole32(data_compressed_size);
    header.string_table_uncompressed_size = YINI::Utils::htole32(uncompressed_strings.size());
    header.string_table_compressed_size = YINI::Utils::htole32(string_compressed_size);

    uint32_t current_offset = sizeof(YINI::Ybin::FileHeader);
    header.hash_table_offset = YINI::Utils::htole32(current_offset);
    current_offset += buckets.size() * sizeof(uint32_t);
    header.entries_offset = YINI::Utils::htole32(current_offset);
    current_offset += entries.size() * sizeof(YINI::Ybin::HashTableEntry);
    header.data_table_offset = YINI::Utils::htole32(current_offset);
    current_offset += compressed_data.size();
    header.string_table_offset = YINI::Utils::htole32(current_offset);

    out.write(reinterpret_cast<const char *>(&header), sizeof(header));
    out.write(reinterpret_cast<const char *>(buckets.data()), buckets.size() * sizeof(uint32_t));

    for (auto &entry : entries)
    {
        entry.key_hash = YINI::Utils::htole64(entry.key_hash);
        entry.key_offset = YINI::Utils::htole32(entry.key_offset);
        entry.value_offset = YINI::Utils::htole32(entry.value_offset);
        out.write(reinterpret_cast<const char *>(&entry), sizeof(entry));
    }

    out.write(compressed_data.data(), compressed_data.size());
    out.write(compressed_strings.data(), compressed_strings.size());

    std::cout << "Successfully cooked " << entries.size() << " entries to " << output_path << std::endl;
}

static void run_file(const char *path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << "Error: Could not open file '" << path << "'" << std::endl;
        return;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    try
    {
        auto ast = parser.parse();
        YINI::YmetaManager ymeta_manager;
        ymeta_manager.load(path);
        YINI::Resolver resolver(ast, ymeta_manager);
        auto resolved_config = resolver.resolve();
        YINI::Validator validator(resolved_config, ast);
        validator.validate();
        ymeta_manager.save(path);
        std::cout << "Validation completed successfully." << std::endl;
    }
    catch (const std::runtime_error &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

// Forward declaration for the new run_file overload
static void run_file(const char *path, std::map<std::string, YINI::YiniVariant> &config_context,
                     YINI::YmetaManager &ymeta_manager);

static void run_prompt()
{
    std::string line;
    YINI::YmetaManager ymeta_manager;
    std::map<std::string, YINI::YiniVariant> config_context;

    for (;;)
    {
        std::cout << "> ";
        if (!std::getline(std::cin, line))
        {
            std::cout << std::endl;
            break;
        }
        if (line.empty())
            continue;

        if (line.rfind(".load ", 0) == 0)
        {
            std::string path = line.substr(6);
            config_context.clear();
            ymeta_manager = YINI::YmetaManager();
            run_file(path.c_str(), config_context, ymeta_manager);
        }
        else if (line.rfind(".get ", 0) == 0)
        {
            std::string key = line.substr(5);
            if (config_context.count(key))
            {
                std::visit(
                    [](auto &&arg)
                    {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<T, std::monostate>)
                        {
                            std::cout << "null";
                        }
                        else if constexpr (std::is_same_v<T, bool>)
                        {
                            std::cout << (arg ? "true" : "false");
                        }
                        else if constexpr (std::is_same_v<T, std::unique_ptr<YINI::YiniArray>>)
                        {
                            std::cout << "[array]";
                        }
                        else if constexpr (std::is_same_v<T, YINI::YiniStruct>)
                        {
                            std::cout << "[struct]";
                        }
                        else if constexpr (std::is_same_v<T, YINI::YiniMap>)
                        {
                            std::cout << "[map]";
                        }
                        else
                        {
                            std::cout << arg;
                        }
                    },
                    config_context[key]);
                std::cout << std::endl;
            }
            else
            {
                std::cout << "null" << std::endl;
            }
        }
        else if (line == ".exit" || line == ".quit")
        {
            break;
        }
        else
        {
            try
            {
                YINI::Lexer lexer(line);
                auto tokens = lexer.scan_tokens();
                YINI::Parser parser(tokens);
                auto ast = parser.parse();
                YINI::Resolver resolver(ast, ymeta_manager);
                auto temp_config = resolver.resolve();
                YINI::Validator validator(temp_config, ast);
                validator.validate();
                std::cout << "Snippet validated successfully." << std::endl;
            }
            catch (const std::runtime_error &e)
            {
                std::cerr << "Error: " << e.what() << std::endl;
            }
        }
    }
}

// Overload run_file to work with an existing context for the REPL
static void run_file(const char *path, std::map<std::string, YINI::YiniVariant> &config_context,
                     YINI::YmetaManager &ymeta_manager)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << "Error: Could not open file '" << path << "'" << std::endl;
        return;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    try
    {
        auto ast = parser.parse();
        ymeta_manager.load(path);
        YINI::Resolver resolver(ast, ymeta_manager);
        config_context = resolver.resolve(); // Load into the provided context
        YINI::Validator validator(config_context, ast);
        validator.validate();
        ymeta_manager.save(path);
        std::cout << "File '" << path << "' loaded and validated." << std::endl;
    }
    catch (const std::runtime_error &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

// Helper to read and parse a file into an AST
static std::vector<std::unique_ptr<YINI::AST::Stmt>> parse_file(const char *path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open file '" + std::string(path) + "'");
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    return parser.parse();
}

static void run_validate_with_schema(const char *schema_path, const char *config_path)
{
    try
    {
        auto schema_ast = parse_file(schema_path);
        auto config_ast = parse_file(config_path);

        // Combine ASTs, with the schema first
        std::vector<std::unique_ptr<YINI::AST::Stmt>> combined_ast;
        std::move(schema_ast.begin(), schema_ast.end(), std::back_inserter(combined_ast));
        std::move(config_ast.begin(), config_ast.end(), std::back_inserter(combined_ast));

        YINI::YmetaManager ymeta_manager; // Not used for this command, but resolver needs it
        YINI::Resolver resolver(combined_ast, ymeta_manager);
        auto resolved_config = resolver.resolve();
        YINI::Validator validator(resolved_config, combined_ast);
        validator.validate();

        std::cout << "File '" << config_path << "' successfully validated against schema '" << schema_path << "'."
                  << std::endl;
    }
    catch (const std::runtime_error &e)
    {
        std::cerr << "Validation failed: " << e.what() << std::endl;
        exit(1);
    }
}

static void run_decompile(const char *path)
{
    try
    {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file.is_open())
        {
            throw std::runtime_error("Could not open .ybin file.");
        }
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        std::vector<char> buffer(size);
        if (!file.read(buffer.data(), size))
        {
            throw std::runtime_error("Could not read .ybin file.");
        }

        const char *base = buffer.data();
        YINI::Ybin::FileHeader header;
        YINI::Ybin::BufferReader::deserialize_header(header, base, size);

        if (header.magic != YINI::Ybin::YBIN_MAGIC || header.version != 2)
        {
            throw std::runtime_error("Invalid or unsupported .ybin file format.");
        }

        std::vector<char> data_table_storage(header.data_table_uncompressed_size);
        LZ4_decompress_safe(base + header.data_table_offset, data_table_storage.data(),
                            header.data_table_compressed_size, header.data_table_uncompressed_size);

        std::vector<char> string_table_storage(header.string_table_uncompressed_size);
        LZ4_decompress_safe(base + header.string_table_offset, string_table_storage.data(),
                            header.string_table_compressed_size, header.string_table_uncompressed_size);

        const char *entries_buffer = base + header.entries_offset;
        const char *string_table = string_table_storage.data();

        std::map<std::string, std::string> decompiled_values;

        for (uint32_t i = 0; i < header.entries_count; ++i)
        {
            YINI::Ybin::HashTableEntry entry;
            YINI::Ybin::BufferReader::deserialize_entry(
                entry, entries_buffer + (i * sizeof(YINI::Ybin::HashTableEntry)), sizeof(YINI::Ybin::HashTableEntry));

            const char *key = string_table + entry.key_offset;
            std::string value_str = "[unknown]";
            switch (entry.value_type)
            {
            case YINI::Ybin::ValueType::Int64:
            {
                YINI::Ybin::BufferReader reader(data_table_storage.data() + entry.value_offset, sizeof(int64_t));
                value_str = std::to_string(static_cast<int64_t>(reader.read_u64_le()));
                break;
            }
            case YINI::Ybin::ValueType::Double:
            {
                YINI::Ybin::BufferReader reader(data_table_storage.data() + entry.value_offset, sizeof(double));
                value_str = std::to_string(reader.read_double_le());
                break;
            }
            case YINI::Ybin::ValueType::Bool:
                value_str = (entry.value_offset != 0) ? "true" : "false";
                break;
            case YINI::Ybin::ValueType::String:
                value_str = "\"" + std::string(string_table + entry.value_offset) + "\"";
                break;
            case YINI::Ybin::ValueType::Color:
            {
                auto c =
                    reinterpret_cast<const YINI::Ybin::ColorData *>(data_table_storage.data() + entry.value_offset);
                value_str =
                    "color(" + std::to_string(c->r) + ", " + std::to_string(c->g) + ", " + std::to_string(c->b) + ")";
                break;
            }
            case YINI::Ybin::ValueType::ArrayInt:
            case YINI::Ybin::ValueType::ArrayDouble:
            case YINI::Ybin::ValueType::ArrayBool:
            case YINI::Ybin::ValueType::ArrayString:
            {
                YINI::Ybin::BufferReader header_reader(data_table_storage.data() + entry.value_offset,
                                                       sizeof(YINI::Ybin::ArrayData));
                uint32_t count = header_reader.read_u32_le();
                const char *array_start =
                    data_table_storage.data() + entry.value_offset + sizeof(YINI::Ybin::ArrayData);
                std::stringstream ss;
                ss << "[";
                if (entry.value_type == YINI::Ybin::ValueType::ArrayInt)
                {
                    YINI::Ybin::BufferReader r(array_start, count * sizeof(int64_t));
                    for (uint32_t i = 0; i < count; ++i)
                        ss << (i > 0 ? ", " : "") << static_cast<int64_t>(r.read_u64_le());
                }
                else if (entry.value_type == YINI::Ybin::ValueType::ArrayDouble)
                {
                    YINI::Ybin::BufferReader r(array_start, count * sizeof(double));
                    for (uint32_t i = 0; i < count; ++i)
                        ss << (i > 0 ? ", " : "") << r.read_double_le();
                }
                else if (entry.value_type == YINI::Ybin::ValueType::ArrayBool)
                {
                    const bool *items = reinterpret_cast<const bool *>(array_start);
                    for (uint32_t i = 0; i < count; ++i)
                        ss << (i > 0 ? ", " : "") << (items[i] ? "true" : "false");
                }
                else
                { // ArrayString
                    YINI::Ybin::BufferReader r(array_start, count * sizeof(uint32_t));
                    for (uint32_t i = 0; i < count; ++i)
                        ss << (i > 0 ? ", " : "") << "\"" << (string_table + r.read_u32_le()) << "\"";
                }
                ss << "]";
                value_str = ss.str();
                break;
            }
            default:
                break;
            }
            decompiled_values[key] = value_str;
        }

        std::string current_section;
        for (const auto &pair : decompiled_values)
        {
            size_t dot_pos = pair.first.find('.');
            if (dot_pos == std::string::npos)
                continue;
            std::string section = pair.first.substr(0, dot_pos);
            std::string key = pair.first.substr(dot_pos + 1);

            if (section != current_section)
            {
                current_section = section;
                std::cout << "\n[" << current_section << "]" << std::endl;
            }
            std::cout << key << " = " << pair.second << std::endl;
        }
    }
    catch (const std::runtime_error &e)
    {
        std::cerr << "Decompilation failed: " << e.what() << std::endl;
        exit(1);
    }
}

int main(int argc, char *argv[])
{
    if (argc > 1 && std::string(argv[1]) == "--server")
    {
        run_language_server();
    }
    else if (argc > 1 && std::string(argv[1]) == "cook")
    {
        std::string output_path;
        std::vector<std::string> input_paths;
        for (int i = 2; i < argc; ++i)
        {
            std::string arg = argv[i];
            if (arg == "-o" && i + 1 < argc)
            {
                output_path = argv[++i];
            }
            else
            {
                input_paths.push_back(arg);
            }
        }

        if (output_path.empty() || input_paths.empty())
        {
            std::cerr << "Usage: yini cook -o <output.ybin> <input1.yini> [input2.yini]..." << std::endl;
            return 64;
        }

        try
        {
            run_cook(output_path, input_paths);
        }
        catch (const std::runtime_error &e)
        {
            std::cerr << "Error during cooking: " << e.what() << std::endl;
            return 1;
        }
    }
    else if (argc == 4 && std::string(argv[1]) == "validate")
    {
        run_validate_with_schema(argv[2], argv[3]);
    }
    else if (argc == 3 && std::string(argv[1]) == "decompile")
    {
        run_decompile(argv[2]);
    }
    else if (argc == 2)
    {
        run_file(argv[1]);
    }
    else if (argc > 2)
    {
        std::cerr << "Usage: yini <command> [args]" << std::endl;
        std::cerr << "Commands: cook, validate, decompile, [script], --server" << std::endl;
        return 64; // EX_USAGE
    }
    else
    {
        run_prompt();
    }

    return 0;
}
