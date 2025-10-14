#include "yini_interop.h"
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "Resolver/Resolver.h"
#include "Resolver/SemanticInfoVisitor.h"
#include "Validator/Validator.h"
#include "Ymeta/YmetaManager.h"
#include "Loader/YbinFormat.h"
#include "Loader/YbinSerialization.h"
#include "Utils/Endian.h"
#include "YiniTypes.h"
#include "lz4.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <any>
#include <cstring>
#include <filesystem>
#include <functional>
#include <variant>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

namespace
{
    thread_local std::string semantic_info_json;

    // Helper to allocate a C-style string for an error message.
    // The caller is responsible for freeing this memory using yini_free_error_string.
    void set_out_error(char** out_error, const std::string& message)
    {
        if (out_error)
        {
            *out_error = new char[message.length() + 1];
            std::strcpy(*out_error, message.c_str());
        }
    }
}

namespace YINI
{
    // A class to manage a loaded .ybin file via memory mapping
    class YbinData
    {
    public:
        YbinData(const std::string& file_path)
        {
#ifdef _WIN32
            m_file_handle = CreateFileA(file_path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (m_file_handle == INVALID_HANDLE_VALUE) throw std::runtime_error("Failed to open .ybin file: " + file_path);
            m_file_size = GetFileSize(m_file_handle, NULL);
            if (m_file_size == INVALID_FILE_SIZE) { CloseHandle(m_file_handle); throw std::runtime_error("Failed to get size of .ybin file."); }
            m_mapping_handle = CreateFileMapping(m_file_handle, NULL, PAGE_READONLY, 0, 0, NULL);
            if (m_mapping_handle == NULL) { CloseHandle(m_file_handle); throw std::runtime_error("Failed to create file mapping for .ybin file."); }
            m_mapped_memory = MapViewOfFile(m_mapping_handle, FILE_MAP_READ, 0, 0, 0);
            if (m_mapped_memory == NULL) { CloseHandle(m_mapping_handle); CloseHandle(m_file_handle); throw std::runtime_error("Failed to memory map .ybin file."); }
#else
            int fd = open(file_path.c_str(), O_RDONLY);
            if (fd == -1) throw std::runtime_error("Failed to open .ybin file: " + file_path);
            struct stat sb;
            if (fstat(fd, &sb) == -1) { close(fd); throw std::runtime_error("Failed to get size of .ybin file."); }
            m_file_size = sb.st_size;
            m_mapped_memory = mmap(NULL, m_file_size, PROT_READ, MAP_PRIVATE, fd, 0);
            close(fd);
            if (m_mapped_memory == MAP_FAILED) throw std::runtime_error("Failed to memory map .ybin file.");
#endif
            const char* base = static_cast<const char*>(m_mapped_memory);
            Ybin::BufferReader::deserialize_header(m_header, base, m_file_size);
            if (m_header.magic != Ybin::YBIN_MAGIC || m_header.version != 2) throw std::runtime_error("Invalid or unsupported .ybin file format.");

            m_raw_buckets_buffer = base + m_header.hash_table_offset;
            m_raw_entries_buffer = base + m_header.entries_offset;

            m_data_table_storage.resize(m_header.data_table_uncompressed_size);
            if (LZ4_decompress_safe(base + m_header.data_table_offset, m_data_table_storage.data(), m_header.data_table_compressed_size, m_header.data_table_uncompressed_size) < 0)
                throw std::runtime_error("Failed to decompress data table.");

            m_string_table_storage.resize(m_header.string_table_uncompressed_size);
            if (LZ4_decompress_safe(base + m_header.string_table_offset, m_string_table_storage.data(), m_header.string_table_compressed_size, m_header.string_table_uncompressed_size) < 0)
                throw std::runtime_error("Failed to decompress string table.");

            m_data_table = m_data_table_storage.data();
            m_string_table = m_string_table_storage.data();
        }

        ~YbinData()
        {
#ifdef _WIN32
            if (m_mapped_memory) UnmapViewOfFile(m_mapped_memory);
            if (m_mapping_handle) CloseHandle(m_mapping_handle);
            if (m_file_handle) CloseHandle(m_file_handle);
#else
            if (m_mapped_memory) munmap(m_mapped_memory, m_file_size);
#endif
        }

        YbinData(const YbinData&) = delete;
        YbinData& operator=(const YbinData&) = delete;

        YiniVariant find(const std::string& key) const {
            uint64_t hash = std::hash<std::string>{}(key);
            uint32_t bucket_index = hash % m_header.hash_table_size;
            Ybin::BufferReader bucket_reader(m_raw_buckets_buffer + (bucket_index * sizeof(uint32_t)), sizeof(uint32_t));
            uint32_t entry_index = bucket_reader.read_u32_le();

            while (entry_index != 0xFFFFFFFF) {
                Ybin::HashTableEntry entry = get_entry(entry_index);
                if (entry.key_hash == hash) {
                    const char* key_from_table = m_string_table + entry.key_offset;
                    if (key == key_from_table) {
                        return decode_value(entry);
                    }
                }
                entry_index = entry.next_entry_index;
            }
            return std::monostate{};
        }

    private:
        Ybin::HashTableEntry get_entry(uint32_t index) const {
            const char* entry_ptr = m_raw_entries_buffer + (index * sizeof(Ybin::HashTableEntry));
            Ybin::HashTableEntry entry;
            Ybin::BufferReader::deserialize_entry(entry, entry_ptr, sizeof(Ybin::HashTableEntry));
            return entry;
        }

        YiniVariant decode_value(const Ybin::HashTableEntry& entry) const {
            switch(entry.value_type) {
                case Ybin::ValueType::Int64: {
                    Ybin::BufferReader reader(m_data_table + entry.value_offset, sizeof(int64_t));
                    return static_cast<int64_t>(reader.read_u64_le());
                }
                case Ybin::ValueType::Double: {
                    Ybin::BufferReader reader(m_data_table + entry.value_offset, sizeof(double));
                    return reader.read_double_le();
                }
                case Ybin::ValueType::Bool:
                    return entry.value_offset != 0;
                case Ybin::ValueType::String:
                    return std::string(m_string_table + entry.value_offset);
                case Ybin::ValueType::Color: {
                    auto c = reinterpret_cast<const Ybin::ColorData*>(m_data_table + entry.value_offset);
                    return ResolvedColor{c->r, c->g, c->b};
                }
                case Ybin::ValueType::ArrayInt:
                case Ybin::ValueType::ArrayDouble:
                case Ybin::ValueType::ArrayBool:
                case Ybin::ValueType::ArrayString: {
                    Ybin::BufferReader header_reader(m_data_table + entry.value_offset, sizeof(Ybin::ArrayData));
                    uint32_t count = header_reader.read_u32_le();
                    auto result = std::make_unique<YiniArray>();
                    result->reserve(count);
                    const char* array_start = m_data_table + entry.value_offset + sizeof(Ybin::ArrayData);

                    if (entry.value_type == Ybin::ValueType::ArrayInt) {
                        Ybin::BufferReader r(array_start, count * sizeof(int64_t));
                        for(uint32_t i=0; i < count; ++i) result->push_back(static_cast<int64_t>(r.read_u64_le()));
                    } else if (entry.value_type == Ybin::ValueType::ArrayDouble) {
                        Ybin::BufferReader r(array_start, count * sizeof(double));
                        for(uint32_t i=0; i < count; ++i) result->push_back(r.read_double_le());
                    } else if (entry.value_type == Ybin::ValueType::ArrayBool) {
                        const bool* items = reinterpret_cast<const bool*>(array_start);
                        for(uint32_t i=0; i < count; ++i) result->push_back(items[i]);
                    } else { // ArrayString
                        Ybin::BufferReader r(array_start, count * sizeof(uint32_t));
                        for(uint32_t i=0; i < count; ++i) result->push_back(std::string(m_string_table + r.read_u32_le()));
                    }
                    return result;
                }
                default:
                    return std::monostate{};
            }
        }

        void* m_mapped_memory = nullptr;
        size_t m_file_size = 0;
#ifdef _WIN32
        HANDLE m_file_handle = NULL;
        HANDLE m_mapping_handle = NULL;
#endif
        Ybin::FileHeader m_header;
        const char* m_raw_buckets_buffer = nullptr;
        const char* m_raw_entries_buffer = nullptr;
        const char* m_data_table = nullptr;
        const char* m_string_table = nullptr;
        std::vector<char> m_data_table_storage;
        std::vector<char> m_string_table_storage;
    };

    class Config
    {
    public:
        static std::unique_ptr<Config> create(const std::string& file_path) {
             if (std::filesystem::path(file_path).extension() == ".ybin") {
                return std::make_unique<Config>(std::make_unique<YbinData>(file_path));
            } else {
                return std::make_unique<Config>(file_path);
            }
        }

        YiniVariant find(const std::string& key) {
            if (m_ybin_data) {
                return m_ybin_data->find(key);
            }
            if (m_resolved_config.count(key)) {
                return m_resolved_config.at(key);
            }
            return std::monostate{};
        }

        void set_value(const std::string& key, YiniVariant value) {
            if (m_ybin_data) {
                // Cannot modify a loaded ybin file in memory
                return;
            }
            m_resolved_config[key] = value;
        }

        void save_to_file(const std::string& file_path) {
            if (m_ybin_data) {
                 throw std::runtime_error("Cannot save a loaded .ybin file.");
            }

            std::ofstream file(file_path);
            if (!file.is_open()) {
                throw std::runtime_error("Could not open file for writing: " + file_path);
            }

            // A simple (and not fully featured) way to write back the config.
            // This doesn't preserve comments, structure, or ordering.
            std::map<std::string, std::map<std::string, YiniVariant>> sections;
            for(const auto& pair : m_resolved_config) {
                size_t dot_pos = pair.first.find('.');
                if (dot_pos == std::string::npos) continue;
                std::string section_name = pair.first.substr(0, dot_pos);
                std::string key_name = pair.first.substr(dot_pos + 1);
                sections[section_name][key_name] = pair.second;
            }

            for(const auto& section_pair : sections) {
                file << "[" << section_pair.first << "]\n";
                for(const auto& key_pair : section_pair.second) {
                    file << key_pair.first << " = ";
                    std::visit([&file](auto&& arg) {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<T, std::monostate>) {
                            file << "null";
                        } else if constexpr (std::is_same_v<T, bool>) {
                            file << (arg ? "true" : "false");
                        } else if constexpr (std::is_same_v<T, std::string>) {
                            file << "\"" << arg << "\"";
                        } else if constexpr (std::is_same_v<T, YiniMap> || std::is_same_v<T, YiniStruct> || std::is_same_v<T, std::unique_ptr<YiniArray>> || std::is_same_v<T, std::unique_ptr<YINI::YiniList>>) {
                            file << "[complex type]"; // Placeholder for now
                        } else {
                            file << arg;
                        }
                    }, key_pair.second);
                    file << "\n";
                }
                file << "\n";
            }
        }

    public:
        Config() = default; // For creating an empty config

        Config(const std::string& file_path) {
            std::ifstream file(file_path);
            if (!file.is_open()) throw std::runtime_error("Could not open file: " + file_path);
            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string source = buffer.str();
            Lexer lexer(source);
            auto tokens = lexer.scan_tokens();
            Parser parser(tokens);
            auto ast = parser.parse();
            m_ymeta_manager.load(file_path);
            Resolver resolver(ast, m_ymeta_manager);
            m_resolved_config = resolver.resolve();
            Validator validator(m_resolved_config, ast);
            validator.validate();
            m_ymeta_manager.save(file_path);
        }

        Config(std::unique_ptr<YbinData> ybin_data) : m_ybin_data(std::move(ybin_data)) {}

    private:
        std::map<std::string, YiniVariant> m_resolved_config;
        YmetaManager m_ymeta_manager;
        std::unique_ptr<YbinData> m_ybin_data;
    };
}

YINI_API void* yini_create_from_file(const char* file_path, char** out_error)
{
    try
    {
        if (out_error) *out_error = nullptr;
        YINI::Config* config = YINI::Config::create(file_path).release();
        return static_cast<void*>(config);
    }
    catch (const std::exception& e)
    {
        set_out_error(out_error, e.what());
        return nullptr;
    }
}

YINI_API void yini_free_error_string(char* error_string)
{
    if (error_string)
    {
        delete[] error_string;
    }
}

YINI_API void yini_destroy(void* handle)
{
    if (handle) delete static_cast<YINI::Config*>(handle);
}

namespace
{
    ValueType get_variant_type(const YINI::YiniVariant& value)
    {
        ValueType type = YINI_TYPE_NULL;
        std::visit([&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::monostate>) {
                type = YINI_TYPE_NULL;
            } else if constexpr (std::is_same_v<T, int64_t>) {
                type = YINI_TYPE_INT;
            } else if constexpr (std::is_same_v<T, double>) {
                type = YINI_TYPE_DOUBLE;
            } else if constexpr (std::is_same_v<T, bool>) {
                type = YINI_TYPE_BOOL;
            } else if constexpr (std::is_same_v<T, std::string>) {
                type = YINI_TYPE_STRING;
            } else if constexpr (std::is_same_v<T, YINI::YiniStruct>) {
                type = YINI_TYPE_STRUCT;
            } else if constexpr (std::is_same_v<T, YINI::YiniMap>) {
                type = YINI_TYPE_MAP;
            } else if constexpr (std::is_same_v<T, std::unique_ptr<YINI::YiniArray>>) {
                if (arg) {
                    const auto& arr = *arg;
                    if (!arr.empty()) {
                        const auto& first = arr.front();
                        if (std::holds_alternative<int64_t>(first)) type = YINI_TYPE_ARRAY_INT;
                        else if (std::holds_alternative<double>(first)) type = YINI_TYPE_ARRAY_DOUBLE;
                        else if (std::holds_alternative<bool>(first)) type = YINI_TYPE_ARRAY_BOOL;
                        else if (std::holds_alternative<std::string>(first)) type = YINI_TYPE_ARRAY_STRING;
                    }
                }
            } else if constexpr (std::is_same_v<T, std::unique_ptr<YINI::YiniList>>) {
                if (arg) {
                    const auto& list = *arg;
                    if (!list.elements.empty()) {
                        const auto& first = list.elements.front();
                        if (std::holds_alternative<int64_t>(first)) type = YINI_TYPE_LIST_INT;
                        else if (std::holds_alternative<double>(first)) type = YINI_TYPE_LIST_DOUBLE;
                        else if (std::holds_alternative<bool>(first)) type = YINI_TYPE_LIST_BOOL;
                        else if (std::holds_alternative<std::string>(first)) type = YINI_TYPE_LIST_STRING;
                    }
                }
            }
        }, value);
        return type;
    }
}

YINI_API ValueType yini_get_type(void* handle, const char* key)
{
    if (!handle || !key) return YINI_TYPE_NULL;
    YINI::Config* config = static_cast<YINI::Config*>(handle);
    YINI::YiniVariant value = config->find(key);
    return get_variant_type(value);
}

template<typename T>
bool get_value(void* handle, const char* key, T* out_value) {
    if (!handle || !key || !out_value) return false;
    YINI::Config* config = static_cast<YINI::Config*>(handle);
    YINI::YiniVariant value = config->find(key);

    bool success = false;
    std::visit([&](auto&& arg) {
        using V = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, int>) {
            if constexpr (std::is_same_v<V, int64_t>) { *out_value = static_cast<int>(arg); success = true; }
            if constexpr (std::is_same_v<V, double>) { *out_value = static_cast<int>(arg); success = true; }
        } else if constexpr (std::is_same_v<T, double>) {
            if constexpr (std::is_same_v<V, int64_t>) { *out_value = static_cast<double>(arg); success = true; }
            if constexpr (std::is_same_v<V, double>) { *out_value = arg; success = true; }
        } else if constexpr (std::is_same_v<T, V>) {
            *out_value = arg;
            success = true;
        }
    }, value);
    return success;
}

YINI_API bool yini_get_int(void* handle, const char* key, int* out_value) {
    return get_value(handle, key, out_value);
}

YINI_API bool yini_get_double(void* handle, const char* key, double* out_value) {
    return get_value(handle, key, out_value);
}

YINI_API bool yini_get_bool(void* handle, const char* key, bool* out_value) {
    return get_value(handle, key, out_value);
}

YINI_API const char* yini_get_string(void* handle, const char* key)
{
    if (!handle || !key) return nullptr;
    YINI::Config* config = static_cast<YINI::Config*>(handle);
    YINI::YiniVariant value = config->find(key);
    if (std::holds_alternative<std::string>(value)) {
        const std::string& str = std::get<std::string>(value);
        char* c_str = new char[str.length() + 1];
        strcpy(c_str, str.c_str());
        return c_str;
    }
    return nullptr;
}

// --- List Getters ---

YINI_API int yini_get_list_size(void* handle, const char* key)
{
    if (!handle || !key) return -1;
    YINI::Config* config = static_cast<YINI::Config*>(handle);
    YINI::YiniVariant value = config->find(key);
    if (std::holds_alternative<std::unique_ptr<YINI::YiniList>>(value)) {
        return static_cast<int>(std::get<std::unique_ptr<YINI::YiniList>>(value)->elements.size());
    }
    return -1;
}

template<typename T>
bool get_list_item(void* handle, const char* key, int index, T* out_value) {
    if (!handle || !key || !out_value) return false;
    YINI::Config* config = static_cast<YINI::Config*>(handle);
    YINI::YiniVariant value = config->find(key);

    if (std::holds_alternative<std::unique_ptr<YINI::YiniList>>(value)) {
        const auto& list = *std::get<std::unique_ptr<YINI::YiniList>>(value);
        if (index >= 0 && static_cast<size_t>(index) < list.elements.size()) {
            const YINI::YiniVariant& item = list.elements.at(index);
            bool success = false;
            std::visit([&](auto&& arg) {
                using V = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, int>) {
                    if constexpr (std::is_same_v<V, int64_t>) { *out_value = static_cast<int>(arg); success = true; }
                    if constexpr (std::is_same_v<V, double>) { *out_value = static_cast<int>(arg); success = true; }
                } else if constexpr (std::is_same_v<T, double>) {
                    if constexpr (std::is_same_v<V, int64_t>) { *out_value = static_cast<double>(arg); success = true; }
                    if constexpr (std::is_same_v<V, double>) { *out_value = arg; success = true; }
                } else if constexpr (std::is_same_v<T, V>) {
                    *out_value = arg;
                    success = true;
                }
            }, item);
            return success;
        }
    }
    return false;
}

YINI_API bool yini_get_list_item_as_int(void* handle, const char* key, int index, int* out_value) {
    return get_list_item(handle, key, index, out_value);
}

YINI_API bool yini_get_list_item_as_double(void* handle, const char* key, int index, double* out_value) {
    return get_list_item(handle, key, index, out_value);
}

YINI_API bool yini_get_list_item_as_bool(void* handle, const char* key, int index, bool* out_value) {
    return get_list_item(handle, key, index, out_value);
}

YINI_API const char* yini_get_list_item_as_string(void* handle, const char* key, int index)
{
    if (!handle || !key) return nullptr;
    YINI::Config* config = static_cast<YINI::Config*>(handle);
    YINI::YiniVariant value = config->find(key);
    if (std::holds_alternative<std::unique_ptr<YINI::YiniList>>(value)) {
        const auto& list = *std::get<std::unique_ptr<YINI::YiniList>>(value);
        if (index >= 0 && static_cast<size_t>(index) < list.elements.size()) {
            const YINI::YiniVariant& item = list.elements.at(index);
            if (std::holds_alternative<std::string>(item)) {
                const std::string& str = std::get<std::string>(item);
                char* c_str = new char[str.length() + 1];
                strcpy(c_str, str.c_str());
                return c_str;
            }
        }
    }
    return nullptr;
}

// --- Write API Implementations ---

YINI_API void* yini_create()
{
    YINI::Config* config = new YINI::Config();
    return static_cast<void*>(config);
}

YINI_API void yini_set_int(void* handle, const char* key, int value)
{
    if (!handle || !key) return;
    YINI::Config* config = static_cast<YINI::Config*>(handle);
    config->set_value(key, static_cast<int64_t>(value));
}

YINI_API void yini_set_double(void* handle, const char* key, double value)
{
    if (!handle || !key) return;
    YINI::Config* config = static_cast<YINI::Config*>(handle);
    config->set_value(key, value);
}

YINI_API void yini_set_bool(void* handle, const char* key, bool value)
{
    if (!handle || !key) return;
    YINI::Config* config = static_cast<YINI::Config*>(handle);
    config->set_value(key, value);
}

YINI_API void yini_set_string(void* handle, const char* key, const char* value)
{
    if (!handle || !key || !value) return;
    YINI::Config* config = static_cast<YINI::Config*>(handle);
    config->set_value(key, std::string(value));
}

YINI_API bool yini_save_to_file(void* handle, const char* file_path, char** out_error)
{
    if (!handle || !file_path) {
        set_out_error(out_error, "Invalid handle or file path provided.");
        return false;
    }
    YINI::Config* config = static_cast<YINI::Config*>(handle);
    try
    {
        if (out_error) *out_error = nullptr;
        config->save_to_file(file_path);
        return true;
    }
    catch (const std::exception& e)
    {
        set_out_error(out_error, e.what());
        return false;
    }
}

YINI_API void yini_free_string(const char* str)
{
    if (str) delete[] str;
}

YINI_API int yini_get_array_size(void* handle, const char* key)
{
    if (!handle || !key) return -1;
    YINI::Config* config = static_cast<YINI::Config*>(handle);
    YINI::YiniVariant value = config->find(key);
    if (std::holds_alternative<std::unique_ptr<YINI::YiniArray>>(value)) {
        return static_cast<int>(std::get<std::unique_ptr<YINI::YiniArray>>(value)->size());
    }
    return -1;
}

template<typename T>
bool get_array_item(void* handle, const char* key, int index, T* out_value) {
    if (!handle || !key || !out_value) return false;
    YINI::Config* config = static_cast<YINI::Config*>(handle);
    YINI::YiniVariant value = config->find(key);

    if (std::holds_alternative<std::unique_ptr<YINI::YiniArray>>(value)) {
        const auto& arr = *std::get<std::unique_ptr<YINI::YiniArray>>(value);
        if (index >= 0 && static_cast<size_t>(index) < arr.size()) {
            const YINI::YiniVariant& item = arr.at(index);
            bool success = false;
            std::visit([&](auto&& arg) {
                using V = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, int>) {
                    if constexpr (std::is_same_v<V, int64_t>) { *out_value = static_cast<int>(arg); success = true; }
                    if constexpr (std::is_same_v<V, double>) { *out_value = static_cast<int>(arg); success = true; }
                } else if constexpr (std::is_same_v<T, double>) {
                    if constexpr (std::is_same_v<V, int64_t>) { *out_value = static_cast<double>(arg); success = true; }
                    if constexpr (std::is_same_v<V, double>) { *out_value = arg; success = true; }
                } else if constexpr (std::is_same_v<T, V>) {
                    *out_value = arg;
                    success = true;
                }
            }, item);
            return success;
        }
    }
    return false;
}

YINI_API bool yini_get_array_item_as_int(void* handle, const char* key, int index, int* out_value) {
    return get_array_item(handle, key, index, out_value);
}

YINI_API bool yini_get_array_item_as_double(void* handle, const char* key, int index, double* out_value) {
    return get_array_item(handle, key, index, out_value);
}

YINI_API bool yini_get_array_item_as_bool(void* handle, const char* key, int index, bool* out_value) {
    return get_array_item(handle, key, index, out_value);
}

YINI_API const char* yini_get_array_item_as_string(void* handle, const char* key, int index)
{
    if (!handle || !key) return nullptr;
    YINI::Config* config = static_cast<YINI::Config*>(handle);
    YINI::YiniVariant value = config->find(key);
    if (std::holds_alternative<std::unique_ptr<YINI::YiniArray>>(value)) {
        const auto& arr = *std::get<std::unique_ptr<YINI::YiniArray>>(value);
        if (index >= 0 && static_cast<size_t>(index) < arr.size()) {
            const YINI::YiniVariant& item = arr.at(index);
            if (std::holds_alternative<std::string>(item)) {
                const std::string& str = std::get<std::string>(item);
                char* c_str = new char[str.length() + 1];
                strcpy(c_str, str.c_str());
                return c_str;
            }
        }
    }
    return nullptr;
}

YINI_API const char* yini_get_semantic_info(const char* source, char** out_error)
{
    try
    {
        if (out_error) *out_error = nullptr;
        YINI::Lexer lexer(source);
        auto tokens = lexer.scan_tokens();
        YINI::Parser parser(tokens);
        auto ast = parser.parse();

        YINI::SemanticInfoVisitor visitor(source, "dummy_uri");
        for (const auto& stmt : ast) {
            if (stmt) stmt->accept(&visitor);
        }

        semantic_info_json = visitor.get_info().dump();
        return semantic_info_json.c_str();
    }
    catch (const std::exception& e)
    {
        set_out_error(out_error, e.what());
        return nullptr;
    }
}

// --- Map Getters ---

YINI_API int yini_get_map_size(void* handle, const char* key)
{
    if (!handle || !key) return -1;
    YINI::Config* config = static_cast<YINI::Config*>(handle);
    YINI::YiniVariant value = config->find(key);
    if (std::holds_alternative<YINI::YiniMap>(value))
    {
        return static_cast<int>(std::get<YINI::YiniMap>(value).size());
    }
    return -1;
}

YINI_API const char* yini_get_map_key_at_index(void* handle, const char* key, int index)
{
    if (!handle || !key || index < 0) return nullptr;
    YINI::Config* config = static_cast<YINI::Config*>(handle);
    YINI::YiniVariant value = config->find(key);
    if (std::holds_alternative<YINI::YiniMap>(value))
    {
        const auto& map = std::get<YINI::YiniMap>(value);
        if (static_cast<size_t>(index) < map.size())
        {
            auto it = map.begin();
            std::advance(it, index);
            const std::string& str_key = it->first;
            char* c_str = new char[str_key.length() + 1];
            strcpy(c_str, str_key.c_str());
            return c_str;
        }
    }
    return nullptr;
}

YINI_API ValueType yini_get_map_value_type(void* handle, const char* key, const char* sub_key)
{
    if (!handle || !key || !sub_key) return YINI_TYPE_NULL;
    YINI::Config* config = static_cast<YINI::Config*>(handle);
    YINI::YiniVariant value = config->find(key);
    if (std::holds_alternative<YINI::YiniMap>(value))
    {
        const auto& map = std::get<YINI::YiniMap>(value);
        if (map.count(sub_key))
        {
            return get_variant_type(map.at(sub_key));
        }
    }
    return YINI_TYPE_NULL;
}

template<typename T>
bool get_map_value(void* handle, const char* key, const char* sub_key, T* out_value) {
    if (!handle || !key || !sub_key || !out_value) return false;
    YINI::Config* config = static_cast<YINI::Config*>(handle);
    YINI::YiniVariant value = config->find(key);

    if (std::holds_alternative<YINI::YiniMap>(value))
    {
        const auto& map = std::get<YINI::YiniMap>(value);
        if (map.count(sub_key))
        {
            const YINI::YiniVariant& sub_value = map.at(sub_key);
            bool success = false;
            std::visit([&](auto&& arg) {
                using V = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, int>) {
                    if constexpr (std::is_same_v<V, int64_t>) { *out_value = static_cast<int>(arg); success = true; }
                    if constexpr (std::is_same_v<V, double>) { *out_value = static_cast<int>(arg); success = true; }
                } else if constexpr (std::is_same_v<T, double>) {
                    if constexpr (std::is_same_v<V, int64_t>) { *out_value = static_cast<double>(arg); success = true; }
                    if constexpr (std::is_same_v<V, double>) { *out_value = arg; success = true; }
                } else if constexpr (std::is_same_v<T, V>) {
                    *out_value = arg;
                    success = true;
                }
            }, sub_value);
            return success;
        }
    }
    return false;
}

YINI_API bool yini_get_map_value_as_int(void* handle, const char* key, const char* sub_key, int* out_value) {
    return get_map_value(handle, key, sub_key, out_value);
}

YINI_API bool yini_get_map_value_as_double(void* handle, const char* key, const char* sub_key, double* out_value) {
    return get_map_value(handle, key, sub_key, out_value);
}

YINI_API bool yini_get_map_value_as_bool(void* handle, const char* key, const char* sub_key, bool* out_value) {
    return get_map_value(handle, key, sub_key, out_value);
}

YINI_API const char* yini_get_map_value_as_string(void* handle, const char* key, const char* sub_key)
{
    if (!handle || !key || !sub_key) return nullptr;
    YINI::Config* config = static_cast<YINI::Config*>(handle);
    YINI::YiniVariant value = config->find(key);
    if (std::holds_alternative<YINI::YiniMap>(value))
    {
        const auto& map = std::get<YINI::YiniMap>(value);
        if (map.count(sub_key))
        {
            const YINI::YiniVariant& sub_value = map.at(sub_key);
            if (std::holds_alternative<std::string>(sub_value))
            {
                const std::string& str = std::get<std::string>(sub_value);
                char* c_str = new char[str.length() + 1];
                strcpy(c_str, str.c_str());
                return c_str;
            }
        }
    }
    return nullptr;
}

// --- Struct Getters ---

YINI_API const char* yini_get_struct_key(void* handle, const char* key)
{
    if (!handle || !key) return nullptr;
    YINI::Config* config = static_cast<YINI::Config*>(handle);
    YINI::YiniVariant value = config->find(key);
    if (std::holds_alternative<YINI::YiniStruct>(value))
    {
        const auto& yini_struct = std::get<YINI::YiniStruct>(value);
        const std::string& str_key = yini_struct.first;
        char* c_str = new char[str_key.length() + 1];
        strcpy(c_str, str_key.c_str());
        return c_str;
    }
    return nullptr;
}

YINI_API ValueType yini_get_struct_value_type(void* handle, const char* key)
{
    if (!handle || !key) return YINI_TYPE_NULL;
    YINI::Config* config = static_cast<YINI::Config*>(handle);
    YINI::YiniVariant value = config->find(key);
    if (std::holds_alternative<YINI::YiniStruct>(value))
    {
        const auto& yini_struct = std::get<YINI::YiniStruct>(value);
        return get_variant_type(*yini_struct.second);
    }
    return YINI_TYPE_NULL;
}

template<typename T>
bool get_struct_value(void* handle, const char* key, T* out_value) {
    if (!handle || !key || !out_value) return false;
    YINI::Config* config = static_cast<YINI::Config*>(handle);
    YINI::YiniVariant value = config->find(key);

    if (std::holds_alternative<YINI::YiniStruct>(value))
    {
        const auto& yini_struct = std::get<YINI::YiniStruct>(value);
        const YINI::YiniVariant& struct_value = *yini_struct.second;

        bool success = false;
        std::visit([&](auto&& arg) {
            using V = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, int>) {
                if constexpr (std::is_same_v<V, int64_t>) { *out_value = static_cast<int>(arg); success = true; }
                if constexpr (std::is_same_v<V, double>) { *out_value = static_cast<int>(arg); success = true; }
            } else if constexpr (std::is_same_v<T, double>) {
                if constexpr (std::is_same_v<V, int64_t>) { *out_value = static_cast<double>(arg); success = true; }
                if constexpr (std::is_same_v<V, double>) { *out_value = arg; success = true; }
            } else if constexpr (std::is_same_v<T, V>) {
                *out_value = arg;
                success = true;
            }
        }, struct_value);
        return success;
    }
    return false;
}

YINI_API bool yini_get_struct_value_as_int(void* handle, const char* key, int* out_value) {
    return get_struct_value(handle, key, out_value);
}

YINI_API bool yini_get_struct_value_as_double(void* handle, const char* key, double* out_value) {
    return get_struct_value(handle, key, out_value);
}

YINI_API bool yini_get_struct_value_as_bool(void* handle, const char* key, bool* out_value) {
    return get_struct_value(handle, key, out_value);
}

YINI_API const char* yini_get_struct_value_as_string(void* handle, const char* key)
{
    if (!handle || !key) return nullptr;
    YINI::Config* config = static_cast<YINI::Config*>(handle);
    YINI::YiniVariant value = config->find(key);
    if (std::holds_alternative<YINI::YiniStruct>(value))
    {
        const auto& yini_struct = std::get<YINI::YiniStruct>(value);
        const YINI::YiniVariant& struct_value = *yini_struct.second;
        if (std::holds_alternative<std::string>(struct_value))
        {
            const std::string& str = std::get<std::string>(struct_value);
            char* c_str = new char[str.length() + 1];
            strcpy(c_str, str.c_str());
            return c_str;
        }
    }
    return nullptr;
}
