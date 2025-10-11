#pragma once

#include <string>
#include <vector>
#include <any>
#include <optional>
#include <cstdint>

namespace YINI
{

// These structs define the binary layout of the .ybin file.
// They are defined here so both the Cooker (in the future) and the Loader can use them.
#pragma pack(push, 1)
struct YbinHeader
{
    char magic[4];
    uint32_t version;
    uint32_t section_table_offset;
    uint32_t num_sections;
};

struct YbinSectionEntry
{
    uint32_t name_offset;
    uint32_t kv_table_offset;
    uint32_t num_kvs;
};

struct YbinKeyValueEntry
{
    uint32_t key_name_offset;
    uint8_t value_type;
    uint32_t value_offset;
};
#pragma pack(pop)


class YbinLoader
{
public:
    YbinLoader(const std::string& path);
    ~YbinLoader();

    std::optional<int> get_int(const std::string& section, const std::string& key);
    std::optional<double> get_double(const std::string& section, const std::string& key);
    std::optional<bool> get_bool(const std::string& section, const std::string& key);
    std::optional<std::string> get_string(const std::string& section, const std::string& key);

private:
    void validate_header();
    const YbinKeyValueEntry* find_key_entry(const std::string& section, const std::string& key);

    const char* m_mapped_data = nullptr;
    size_t m_file_size = 0;

#ifdef _WIN32
    void* m_file_handle = nullptr;
    void* m_map_handle = nullptr;
#endif
};

} // namespace YINI
