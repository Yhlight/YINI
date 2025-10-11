#include "yini_interop.h"
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "Resolver/Resolver.h"
#include "Validator/Validator.h"
#include "Ymeta/YmetaManager.h"
#include "Loader/YbinV2Format.h"
#include "YiniTypes.h"

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
    thread_local std::string last_error_message;

    void set_last_error(const std::string& message)
    {
        last_error_message = message;
    }
}

namespace YINI
{
    // A class to manage a loaded .ybin V2 file via memory mapping
    class YbinData
    {
    public:
        YbinData(const std::string& file_path)
        {
#ifdef _WIN32
            m_file_handle = CreateFileA(file_path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (m_file_handle == INVALID_HANDLE_VALUE) {
                throw std::runtime_error("Failed to open .ybin file: " + file_path);
            }

            m_file_size = GetFileSize(m_file_handle, NULL);
            if (m_file_size == INVALID_FILE_SIZE) {
                CloseHandle(m_file_handle);
                throw std::runtime_error("Failed to get size of .ybin file.");
            }

            m_mapping_handle = CreateFileMapping(m_file_handle, NULL, PAGE_READONLY, 0, 0, NULL);
            if (m_mapping_handle == NULL) {
                CloseHandle(m_file_handle);
                throw std::runtime_error("Failed to create file mapping for .ybin file.");
            }

            m_mapped_memory = MapViewOfFile(m_mapping_handle, FILE_MAP_READ, 0, 0, 0);
            if (m_mapped_memory == NULL) {
                CloseHandle(m_mapping_handle);
                CloseHandle(m_file_handle);
                throw std::runtime_error("Failed to memory map .ybin file.");
            }
#else
            int fd = open(file_path.c_str(), O_RDONLY);
            if (fd == -1) {
                throw std::runtime_error("Failed to open .ybin file: " + file_path);
            }

            struct stat sb;
            if (fstat(fd, &sb) == -1) {
                close(fd);
                throw std::runtime_error("Failed to get size of .ybin file.");
            }
            m_file_size = sb.st_size;

            m_mapped_memory = mmap(NULL, m_file_size, PROT_READ, MAP_PRIVATE, fd, 0);
            close(fd);

            if (m_mapped_memory == MAP_FAILED) {
                throw std::runtime_error("Failed to memory map .ybin file.");
            }
#endif
            const char* base = static_cast<const char*>(m_mapped_memory);
            m_header = reinterpret_cast<const YbinV2::FileHeader*>(base);
            if (m_header->magic != YbinV2::YBIN_V2_MAGIC || m_header->version != 2) {
                throw std::runtime_error("Invalid or unsupported .ybin file format.");
            }

            m_buckets = reinterpret_cast<const uint32_t*>(base + m_header->hash_table_offset);
            m_entries = reinterpret_cast<const YbinV2::HashTableEntry*>(base + m_header->entries_offset);
            m_data_table = base + m_header->data_table_offset;
            m_string_table = base + m_header->string_table_offset;
        }

        ~YbinData()
        {
#ifdef _WIN32
            if (m_mapped_memory) UnmapViewOfFile(m_mapped_memory);
            if (m_mapping_handle) CloseHandle(m_mapping_handle);
            if (m_file_handle) CloseHandle(m_file_handle);
#else
            if (m_mapped_memory) {
                munmap(m_mapped_memory, m_file_size);
            }
#endif
        }

        YbinData(const YbinData&) = delete;
        YbinData& operator=(const YbinData&) = delete;

        std::any find(const std::string& key) const {
            uint64_t hash = std::hash<std::string>{}(key);
            uint32_t bucket_index = hash % m_header->hash_table_size;
            uint32_t entry_index = m_buckets[bucket_index];

            while (entry_index != 0xFFFFFFFF) {
                const auto& entry = m_entries[entry_index];
                if (entry.key_hash == hash) {
                    const char* key_from_table = m_string_table + entry.key_offset;
                    if (key == key_from_table) {
                        return decode_value(entry);
                    }
                }
                entry_index = entry.next_entry_index;
            }
            return {};
        }

    private:
        std::any decode_value(const YbinV2::HashTableEntry& entry) const {
            switch(entry.value_type) {
                case YbinV2::ValueType::Double:
                    return *reinterpret_cast<const double*>(m_data_table + entry.value_offset);
                case YbinV2::ValueType::Bool:
                    return entry.value_offset != 0;
                case YbinV2::ValueType::String:
                    return std::string(m_string_table + entry.value_offset);
                case YbinV2::ValueType::Color: {
                    auto c = reinterpret_cast<const YbinV2::ColorData*>(m_data_table + entry.value_offset);
                    return ResolvedColor{c->r, c->g, c->b};
                }
                case YbinV2::ValueType::ArrayDouble:
                case YbinV2::ValueType::ArrayBool:
                case YbinV2::ValueType::ArrayString: {
                    const auto* arr_header = reinterpret_cast<const YbinV2::ArrayData*>(m_data_table + entry.value_offset);
                    std::vector<std::any> result;
                    result.reserve(arr_header->count);

                    uint32_t header_end_addr = entry.value_offset + sizeof(YINI::YbinV2::ArrayData);
                    size_t element_alignment = 1;
                    if (entry.value_type == YINI::YbinV2::ValueType::ArrayDouble) element_alignment = alignof(double);
                    else if (entry.value_type == YINI::YbinV2::ValueType::ArrayBool) element_alignment = alignof(bool);
                    else if (entry.value_type == YINI::YbinV2::ValueType::ArrayString) element_alignment = alignof(uint32_t);

                    size_t padding = (element_alignment - (header_end_addr % element_alignment)) % element_alignment;
                    const void* arr_start = m_data_table + header_end_addr + padding;

                    if (entry.value_type == YbinV2::ValueType::ArrayDouble) {
                        const double* items = static_cast<const double*>(arr_start);
                        for(uint32_t i=0; i < arr_header->count; ++i) result.push_back(items[i]);
                    } else if (entry.value_type == YINI::YbinV2::ValueType::ArrayBool) {
                        const bool* items = static_cast<const bool*>(arr_start);
                        for(uint32_t i=0; i < arr_header->count; ++i) result.push_back(items[i]);
                    } else { // ArrayString
                        const uint32_t* items = static_cast<const uint32_t*>(arr_start);
                        for(uint32_t i=0; i < arr_header->count; ++i) result.push_back(std::string(m_string_table + items[i]));
                    }
                    return result;
                }
                default:
                    return {};
            }
        }

        void* m_mapped_memory = nullptr;
        size_t m_file_size = 0;
#ifdef _WIN32
        HANDLE m_file_handle = NULL;
        HANDLE m_mapping_handle = NULL;
#endif
        const YbinV2::FileHeader* m_header = nullptr;
        const uint32_t* m_buckets = nullptr;
        const YbinV2::HashTableEntry* m_entries = nullptr;
        const char* m_data_table = nullptr;
        const char* m_string_table = nullptr;
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

        std::any find(const std::string& key) {
            if (m_ybin_data) {
                return m_ybin_data->find(key);
            }
            if (m_resolved_config.count(key)) {
                return m_resolved_config.at(key);
            }
            return {};
        }

    public:
        Config(const std::string& file_path) {
            std::ifstream file(file_path);
            if (!file.is_open()) {
                throw std::runtime_error("Could not open file: " + file_path);
            }

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
        std::map<std::string, std::any> m_resolved_config;
        YmetaManager m_ymeta_manager;
        std::unique_ptr<YbinData> m_ybin_data;
    };
}

YINI_API void* yini_create_from_file(const char* file_path)
{
    try
    {
        set_last_error("");
        YINI::Config* config = YINI::Config::create(file_path).release();
        return static_cast<void*>(config);
    }
    catch (const std::exception& e)
    {
        set_last_error(e.what());
        return nullptr;
    }
}

YINI_API const char* yini_get_last_error() { return last_error_message.c_str(); }

YINI_API void yini_destroy(void* handle)
{
    if (handle) delete static_cast<YINI::Config*>(handle);
}

// Generic helper for yini_get_* functions
template<typename T>
bool get_value(void* handle, const char* key, T* out_value) {
    if (!handle || !key || !out_value) return false;
    YINI::Config* config = static_cast<YINI::Config*>(handle);
    try {
        std::any value = config->find(key);
        if (value.has_value()) {
            // Allow cast from double to int for convenience
            if constexpr (std::is_same_v<T, int>) {
                 if (value.type() == typeid(double)) {
                    *out_value = static_cast<int>(std::any_cast<double>(value));
                    return true;
                 }
            }
            *out_value = std::any_cast<T>(value);
            return true;
        }
    } catch (const std::bad_any_cast&) {}
    return false;
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
    try {
        std::any value = config->find(key);
        if (value.has_value()) {
            const std::string& str = std::any_cast<const std::string&>(value);
            char* c_str = new char[str.length() + 1];
            strcpy(c_str, str.c_str());
            return c_str;
        }
    } catch (const std::bad_any_cast&) {}
    return nullptr;
}

YINI_API void yini_free_string(const char* str)
{
    if (str) delete[] str;
}

YINI_API int yini_get_array_size(void* handle, const char* key)
{
    if (!handle || !key) return -1;
    YINI::Config* config = static_cast<YINI::Config*>(handle);
    try {
        std::any value = config->find(key);
        if (value.has_value()) {
            const auto& vec = std::any_cast<const std::vector<std::any>&>(value);
            return static_cast<int>(vec.size());
        }
    } catch (const std::bad_any_cast&) {}
    return -1;
}

// Generic helper for yini_get_array_item_as_* functions
template<typename T>
bool get_array_item(void* handle, const char* key, int index, T* out_value) {
    if (!handle || !key || !out_value) return false;
    YINI::Config* config = static_cast<YINI::Config*>(handle);
    try {
        std::any value = config->find(key);
        if (value.has_value()) {
            const auto& vec = std::any_cast<const std::vector<std::any>&>(value);
            if (index >= 0 && static_cast<size_t>(index) < vec.size()) {
                 if constexpr (std::is_same_v<T, int>) {
                     if (vec.at(index).type() == typeid(double)) {
                        *out_value = static_cast<int>(std::any_cast<double>(vec.at(index)));
                        return true;
                     }
                 }
                *out_value = std::any_cast<T>(vec.at(index));
                return true;
            }
        }
    } catch (const std::bad_any_cast&) {}
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
    try {
        std::any value = config->find(key);
        if (value.has_value()) {
            const auto& vec = std::any_cast<const std::vector<std::any>&>(value);
            if (index >= 0 && static_cast<size_t>(index) < vec.size()) {
                const std::string& str = std::any_cast<const std::string&>(vec.at(index));
                char* c_str = new char[str.length() + 1];
                strncpy(c_str, str.c_str(), str.length() + 1);
                return c_str;
            }
        }
    } catch (const std::bad_any_cast&) {}
    return nullptr;
}
