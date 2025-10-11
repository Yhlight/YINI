#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <variant>
#include <cmath>
#include "lz4.h"

#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "Resolver/Resolver.h"
#include "Resolver/SemanticInfoVisitor.h"
#include "Ymeta/YmetaManager.h"
#include "Validator/Validator.h"
#include "Loader/YbinFormat.h"
#include "Utils/Endian.h"
#include "YiniTypes.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

void log_message(const std::string& msg) {
    std::cerr << "YINI LS: " << msg << std::endl;
}

void send_json_rpc(const json& msg) {
    std::string dumped_msg = msg.dump();
    std::cout << "Content-Length: " << dumped_msg.length() << "\r\n\r\n" << dumped_msg << std::flush;
    log_message("Sent: " + dumped_msg);
}

void run_language_server() {
    log_message("Language server started.");

    while (std::cin.good()) {
        std::string line;
        long length = 0;
        while (std::getline(std::cin, line) && !line.empty() && line != "\r") {
            if (line.rfind("Content-Length: ", 0) == 0) {
                length = std::stol(line.substr(16));
            }
        }

        if (length == 0) {
            continue;
        }

        std::string content(length, '\0');
        std::cin.read(&content[0], length);

        log_message("Received: " + content);

        json request = json::parse(content);

        json response;
        response["id"] = request["id"];
        response["jsonrpc"] = "2.0";

        if (request["method"] == "initialize") {
            response["result"]["capabilities"]["semanticTokensProvider"]["legend"] = {
                {"tokenTypes", {"string", "number", "operator", "macro", "namespace", "property", "variable", "class"}},
                {"tokenModifiers", {"readonly"}}
            };
            response["result"]["capabilities"]["semanticTokensProvider"]["full"] = true;
            send_json_rpc(response);
        } else if (request["method"] == "textDocument/semanticTokens/full") {
            std::string uri = request["params"]["textDocument"]["uri"];
            // In a real LS, you'd read the file content from the URI
            // For now, we'll assume the client sends the text, which is more common
            // but this is a simplified example. We'll use a placeholder.
             std::string text = ""; // This needs to be fetched based on VSCode's text sync events

            // This part is tricky as we don't have the text. A full LS needs text document sync.
            // Let's assume for now we can get it, and proceed.
            // In a real implementation, we'd have a document manager.

            response["result"]["data"] = json::array(); // Return empty for now.
             send_json_rpc(response);

        } else if (request["method"] == "shutdown") {
            response["result"] = nullptr;
            send_json_rpc(response);
        } else if (request["method"] == "exit") {
            return;
        }
    }
}


// Helper class to build the string table
class StringTableBuilder {
public:
    uint32_t add(const std::string& s) {
        if (m_offsets.count(s)) {
            return m_offsets[s];
        }
        uint32_t offset = m_blob.size();
        m_blob.append(s).push_back('\0');
        m_offsets[s] = offset;
        return offset;
    }

    const std::string& get_blob() const {
        return m_blob;
    }

private:
    std::map<std::string, uint32_t> m_offsets;
    std::string m_blob;
};

// Helper class to build the data table, ensuring alignment
class DataTableBuilder {
public:
    uint32_t add(const void* data, size_t size, size_t alignment = 8) {
        // Add padding to ensure alignment
        size_t current_offset = m_blob.size();
        size_t padding = (alignment - (current_offset % alignment)) % alignment;
        m_blob.insert(m_blob.end(), padding, 0);

        uint32_t offset = m_blob.size();
        const char* bytes = static_cast<const char*>(data);
        m_blob.insert(m_blob.end(), bytes, bytes + size);
        return offset;
    }

    template<typename T>
    uint32_t add(T value) {
        if constexpr (std::is_arithmetic_v<T> && sizeof(T) > 1) {
             if constexpr (sizeof(T) == 2) value = YINI::Utils::htole16(value);
             if constexpr (sizeof(T) == 4) value = YINI::Utils::htole32(value);
             if constexpr (sizeof(T) == 8) value = YINI::Utils::htole64(value);
        }
        return add(&value, sizeof(T), alignof(T));
    }

    const std::vector<char>& get_blob() const {
        return m_blob;
    }

private:
    std::vector<char> m_blob;
};

// Helper to check if a double can be safely represented as an int64_t
bool is_integer_value(double d, int64_t& out_int) {
    if (std::fmod(d, 1.0) != 0.0) {
        return false;
    }
    if (d < static_cast<double>(std::numeric_limits<int64_t>::min()) || d > static_cast<double>(std::numeric_limits<int64_t>::max())) {
        return false;
    }
    out_int = static_cast<int64_t>(d);
    return true;
}


static YINI::Ybin::ValueType get_array_value_type(const std::vector<std::any>& arr) {
    if (arr.empty()) return YINI::Ybin::ValueType::Null; // Or a specific empty array type

    const std::type_info& first_type = arr.front().type();

    if (first_type == typeid(double)) {
        // Check if all elements are integers
        bool all_integers = true;
        for(const auto& item : arr) {
            int64_t dummy;
            if (item.type() != typeid(double) || !is_integer_value(std::any_cast<double>(item), dummy)) {
                all_integers = false;
                break;
            }
        }
        if (all_integers) return YINI::Ybin::ValueType::ArrayInt;
        return YINI::Ybin::ValueType::ArrayDouble;
    }

    if (first_type == typeid(bool)) return YINI::Ybin::ValueType::ArrayBool;
    if (first_type == typeid(std::string)) return YINI::Ybin::ValueType::ArrayString;

    return YINI::Ybin::ValueType::Null; // Unsupported array type
}

static void run_cook(const std::string& output_path, const std::vector<std::string>& input_paths) {
    std::map<std::string, std::any> combined_config;

    // 1. Parse and resolve all input files, merging them
    for (const auto& path : input_paths) {
        std::ifstream file(path);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open input file: " + path);
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string source = buffer.str();

        YINI::Lexer lexer(source);
        auto tokens = lexer.scan_tokens();
        YINI::Parser parser(tokens);
        auto ast = parser.parse();

        // Use a dummy YmetaManager for cooking
        YINI::YmetaManager ymeta_manager;
        YINI::Resolver resolver(ast, ymeta_manager);
        auto resolved_config = resolver.resolve();

        // Merge into the combined config
        combined_config.insert(resolved_config.begin(), resolved_config.end());
    }

    // 2. Build the data structures for the ybin file
    StringTableBuilder string_table;
    DataTableBuilder data_table;
    std::vector<YINI::Ybin::HashTableEntry> entries;

    for (const auto& pair : combined_config) {
        const std::string& key = pair.first;
        const std::any& value = pair.second;

        YINI::Ybin::HashTableEntry entry;
        entry.key_hash = std::hash<std::string>{}(key);
        entry.key_offset = string_table.add(key);
        entry.next_entry_index = 0xFFFFFFFF; // Sentinel for end of chain

        const auto& type = value.type();
        int64_t int_val;

        if (type == typeid(double) && is_integer_value(std::any_cast<double>(value), int_val)) {
            entry.value_type = YINI::Ybin::ValueType::Int64;
            entry.value_offset = data_table.add(int_val);
        } else if (type == typeid(double)) {
            entry.value_type = YINI::Ybin::ValueType::Double;
            entry.value_offset = data_table.add(std::any_cast<double>(value));
        } else if (type == typeid(bool)) {
            entry.value_type = YINI::Ybin::ValueType::Bool;
            // Store bool directly in offset
            entry.value_offset = std::any_cast<bool>(value) ? 1 : 0;
        } else if (type == typeid(std::string)) {
            entry.value_type = YINI::Ybin::ValueType::String;
            entry.value_offset = string_table.add(std::any_cast<std::string>(value));
        } else if (type == typeid(YINI::ResolvedColor)) {
            entry.value_type = YINI::Ybin::ValueType::Color;
            auto c = std::any_cast<YINI::ResolvedColor>(value);
            YINI::Ybin::ColorData cdata{c.r, c.g, c.b};
            entry.value_offset = data_table.add(cdata);
        } else if (type == typeid(std::vector<std::any>)) {
            const auto& arr = std::any_cast<std::vector<std::any>>(value);
            entry.value_type = get_array_value_type(arr);

            YINI::Ybin::ArrayData array_header{ (uint32_t)arr.size() };
            uint32_t header_offset = data_table.add(array_header);
            entry.value_offset = header_offset;

            if (entry.value_type == YINI::Ybin::ValueType::ArrayInt) {
                for(const auto& item : arr) {
                     is_integer_value(std::any_cast<double>(item), int_val);
                     data_table.add(int_val);
                }
            } else if (entry.value_type == YINI::Ybin::ValueType::ArrayDouble) {
                 for(const auto& item : arr) data_table.add(std::any_cast<double>(item));
            } else if (entry.value_type == YINI::Ybin::ValueType::ArrayBool) {
                 for(const auto& item : arr) data_table.add(std::any_cast<bool>(item));
            } else if (entry.value_type == YINI::Ybin::ValueType::ArrayString) {
                 for(const auto& item : arr) {
                    uint32_t str_offset = string_table.add(std::any_cast<std::string>(item));
                    data_table.add(str_offset);
                 }
            }
        }
        else {
            entry.value_type = YINI::Ybin::ValueType::Null;
            entry.value_offset = 0;
        }

        if (entry.value_type != YINI::Ybin::ValueType::Null) {
            entries.push_back(entry);
        }
    }

    // 3. Build the hash table buckets
    size_t bucket_count = entries.size() * 1.5;
    if (bucket_count == 0) bucket_count = 1;
    std::vector<uint32_t> buckets(bucket_count, 0xFFFFFFFF);

    for (uint32_t i = 0; i < entries.size(); ++i) {
        uint64_t key_hash_le = YINI::Utils::htole64(entries[i].key_hash);
        uint32_t bucket_index = key_hash_le % bucket_count;

        if (buckets[bucket_index] == 0xFFFFFFFF) {
            buckets[bucket_index] = YINI::Utils::htole32(i);
        } else {
            // Prepend to the chain
            entries[i].next_entry_index = buckets[bucket_index];
            buckets[bucket_index] = YINI::Utils::htole32(i);
        }
    }

    // 4. Compress data and string tables
    const auto& uncompressed_data = data_table.get_blob();
    int data_max_size = LZ4_compressBound(uncompressed_data.size());
    std::vector<char> compressed_data(data_max_size);
    int data_compressed_size = LZ4_compress_default(uncompressed_data.data(), compressed_data.data(), uncompressed_data.size(), data_max_size);
    compressed_data.resize(data_compressed_size);

    const auto& uncompressed_strings = string_table.get_blob();
    int string_max_size = LZ4_compressBound(uncompressed_strings.size());
    std::vector<char> compressed_strings(string_max_size);
    int string_compressed_size = LZ4_compress_default(uncompressed_strings.data(), compressed_strings.data(), uncompressed_strings.size(), string_max_size);
    compressed_strings.resize(string_compressed_size);


    // 5. Write everything to the file
    std::ofstream out(output_path, std::ios::binary);
    if (!out) {
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

    out.write(reinterpret_cast<const char*>(&header), sizeof(header));
    out.write(reinterpret_cast<const char*>(buckets.data()), buckets.size() * sizeof(uint32_t));

    // Write entries one by one after converting to little-endian
    for(auto& entry : entries) {
        entry.key_hash = YINI::Utils::htole64(entry.key_hash);
        entry.key_offset = YINI::Utils::htole32(entry.key_offset);
        entry.value_offset = YINI::Utils::htole32(entry.value_offset);
        // next_entry_index is already LE from the bucket-building step
        out.write(reinterpret_cast<const char*>(&entry), sizeof(entry));
    }

    out.write(compressed_data.data(), compressed_data.size());
    out.write(compressed_strings.data(), compressed_strings.size());

    std::cout << "Successfully cooked " << entries.size() << " entries to " << output_path << std::endl;
}


static void run_file(const char* path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file '" << path << "'" << std::endl;
        return;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    try {
        auto ast = parser.parse();
        YINI::YmetaManager ymeta_manager;
        ymeta_manager.load(path);
        YINI::Resolver resolver(ast, ymeta_manager);
        auto resolved_config = resolver.resolve();
        YINI::Validator validator(resolved_config, ast);
        validator.validate();
        ymeta_manager.save(path);
        std::cout << "Validation completed successfully." << std::endl;
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

// Forward declaration for the new run_file overload
static void run_file(const char* path, std::map<std::string, std::any>& config_context, YINI::YmetaManager& ymeta_manager);

static void run_prompt() {
    std::string line;
    YINI::YmetaManager ymeta_manager;
    std::map<std::string, std::any> config_context;

    for (;;) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) {
            std::cout << std::endl;
            break;
        }
        if (line.empty()) continue;

        if (line.rfind(".load ", 0) == 0) {
            std::string path = line.substr(6);
            config_context.clear();
            ymeta_manager = YINI::YmetaManager();
            run_file(path.c_str(), config_context, ymeta_manager);
        } else if (line.rfind(".get ", 0) == 0) {
            std::string key = line.substr(5);
            if (config_context.count(key)) {
                // This is a simplified output. A real implementation would handle different types.
                try {
                    if (config_context[key].type() == typeid(double)) {
                        std::cout << std::any_cast<double>(config_context[key]) << std::endl;
                    } else if (config_context[key].type() == typeid(std::string)) {
                        std::cout << std::any_cast<std::string>(config_context[key]) << std::endl;
                    } else if (config_context[key].type() == typeid(bool)) {
                        std::cout << (std::any_cast<bool>(config_context[key]) ? "true" : "false") << std::endl;
                    } else {
                         std::cout << "[complex type]" << std::endl;
                    }
                } catch (const std::bad_any_cast& e) {
                     std::cerr << "Error: Could not cast value." << std::endl;
                }
            } else {
                std::cout << "null" << std::endl;
            }
        } else if (line == ".exit" || line == ".quit") {
            break;
        }
        else
        {
             try {
                YINI::Lexer lexer(line);
                auto tokens = lexer.scan_tokens();
                YINI::Parser parser(tokens);
                auto ast = parser.parse();
                YINI::Resolver resolver(ast, ymeta_manager);
                auto temp_config = resolver.resolve();
                // We don't merge this into the main context, just validate it.
                YINI::Validator validator(temp_config, ast);
                validator.validate();
                std::cout << "Snippet validated successfully." << std::endl;
            } catch (const std::runtime_error& e) {
                std::cerr << "Error: " << e.what() << std::endl;
            }
        }
    }
}

// Overload run_file to work with an existing context for the REPL
static void run_file(const char* path, std::map<std::string, std::any>& config_context, YINI::YmetaManager& ymeta_manager) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file '" << path << "'" << std::endl;
        return;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    try {
        auto ast = parser.parse();
        ymeta_manager.load(path);
        YINI::Resolver resolver(ast, ymeta_manager);
        config_context = resolver.resolve(); // Load into the provided context
        YINI::Validator validator(config_context, ast);
        validator.validate();
        ymeta_manager.save(path);
        std::cout << "File '" << path << "' loaded and validated." << std::endl;
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc == 2 && std::string(argv[1]) == "--server") {
        run_language_server();
    }
    else if (argc > 1 && std::string(argv[1]) == "cook") {
        std::string output_path;
        std::vector<std::string> input_paths;
        for (int i = 2; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "-o" && i + 1 < argc) {
                output_path = argv[++i];
            } else {
                input_paths.push_back(arg);
            }
        }

        if (output_path.empty() || input_paths.empty()) {
            std::cerr << "Usage: yini cook -o <output.ybin> <input1.yini> [input2.yini]..." << std::endl;
            return 64;
        }

        try {
            run_cook(output_path, input_paths);
        } catch (const std::runtime_error& e) {
            std::cerr << "Error during cooking: " << e.what() << std::endl;
            return 1;
        }
    } else if (argc > 2 && (std::string(argv[1]) != "cook")) {
        std::cout << "Usage: yini [script]" << std::endl;
        return 64; // EX_USAGE
    } else if (argc == 2) {
        run_file(argv[1]);
    } else {
        run_prompt();
    }

    return 0;
}
