#include "YbinLoader.h"
#include <stdexcept>
#include <cstring>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif
#include <vector>
#include <cstdint>

namespace YINI
{

#ifdef _WIN32
YbinLoader::YbinLoader(const std::string& path)
{
    m_file_handle = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (m_file_handle == INVALID_HANDLE_VALUE)
    {
        throw std::runtime_error("Failed to open file: " + path);
    }

    LARGE_INTEGER file_size;
    if (!GetFileSizeEx(m_file_handle, &file_size))
    {
        CloseHandle(m_file_handle);
        throw std::runtime_error("Failed to get file size.");
    }
    m_file_size = file_size.QuadPart;

    m_map_handle = CreateFileMappingA(m_file_handle, NULL, PAGE_READONLY, 0, 0, NULL);
    if (m_map_handle == NULL)
    {
        CloseHandle(m_file_handle);
        throw std::runtime_error("Failed to create file mapping.");
    }

    m_mapped_data = static_cast<const char*>(MapViewOfFile(m_map_handle, FILE_MAP_READ, 0, 0, m_file_size));
    if (m_mapped_data == NULL)
    {
        CloseHandle(m_map_handle);
        CloseHandle(m_file_handle);
        throw std::runtime_error("Failed to map view of file.");
    }
    validate_header();
}

YbinLoader::~YbinLoader()
{
    if (m_mapped_data) UnmapViewOfFile(m_mapped_data);
    if (m_map_handle) CloseHandle(m_map_handle);
    if (m_file_handle) CloseHandle(m_file_handle);
}

#else // POSIX implementation
YbinLoader::YbinLoader(const std::string& path)
{
    int fd = open(path.c_str(), O_RDONLY);
    if (fd == -1)
    {
        throw std::runtime_error("Failed to open file: " + path);
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1)
    {
        close(fd);
        throw std::runtime_error("Failed to get file size.");
    }
    m_file_size = sb.st_size;

    m_mapped_data = static_cast<const char*>(mmap(NULL, m_file_size, PROT_READ, MAP_PRIVATE, fd, 0));
    close(fd);

    if (m_mapped_data == MAP_FAILED)
    {
        throw std::runtime_error("Failed to map file to memory.");
    }
    validate_header();
}

YbinLoader::~YbinLoader()
{
    if (m_mapped_data)
    {
        munmap(const_cast<char*>(m_mapped_data), m_file_size);
    }
}
#endif

void YbinLoader::validate_header()
{
    if (m_file_size < sizeof(YbinHeader))
    {
        throw std::runtime_error("File is too small to be a valid .ybin file.");
    }

    const auto* header = reinterpret_cast<const YbinHeader*>(m_mapped_data);
    if (std::strncmp(header->magic, "YINI", 4) != 0)
    {
        throw std::runtime_error("Invalid .ybin file magic number.");
    }
    if (header->version != 1)
    {
        throw std::runtime_error("Unsupported .ybin file version.");
    }
}

const YbinKeyValueEntry* YbinLoader::find_key_entry(const std::string& section, const std::string& key)
{
    const auto* header = reinterpret_cast<const YbinHeader*>(m_mapped_data);
    const auto* sections = reinterpret_cast<const YbinSectionEntry*>(m_mapped_data + header->section_table_offset);

    // Find the section
    for (uint32_t i = 0; i < header->num_sections; ++i)
    {
        const char* section_name = m_mapped_data + sections[i].name_offset;
        if (section == section_name)
        {
            // Find the key in this section
            const auto* kvs = reinterpret_cast<const YbinKeyValueEntry*>(m_mapped_data + sections[i].kv_table_offset);
            for (uint32_t j = 0; j < sections[i].num_kvs; ++j)
            {
                const char* key_name = m_mapped_data + kvs[j].key_name_offset;
                if (key == key_name)
                {
                    return &kvs[j];
                }
            }
            return nullptr; // Key not found in this section
        }
    }
    return nullptr; // Section not found
}

std::optional<int> YbinLoader::get_int(const std::string& section, const std::string& key)
{
    const YbinKeyValueEntry* entry = find_key_entry(section, key);
    if (!entry) return std::nullopt;

    if (entry->value_type == 0x01) { // INT
        return *reinterpret_cast<const int*>(m_mapped_data + entry->value_offset);
    }
    if (entry->value_type == 0x02) { // FLOAT
        // Allow safe conversion from double to int
        double double_val = *reinterpret_cast<const double*>(m_mapped_data + entry->value_offset);
        return static_cast<int>(double_val);
    }
    return std::nullopt;
}

std::optional<double> YbinLoader::get_double(const std::string& section, const std::string& key)
{
    const YbinKeyValueEntry* entry = find_key_entry(section, key);
    if (!entry) return std::nullopt;

    if (entry->value_type == 0x02) { // FLOAT
        return *reinterpret_cast<const double*>(m_mapped_data + entry->value_offset);
    }
    if (entry->value_type == 0x01) { // INT
        // Allow conversion from int to double
        int int_val = *reinterpret_cast<const int*>(m_mapped_data + entry->value_offset);
        return static_cast<double>(int_val);
    }
    return std::nullopt;
}

std::optional<bool> YbinLoader::get_bool(const std::string& section, const std::string& key)
{
    const YbinKeyValueEntry* entry = find_key_entry(section, key);
    if (entry && entry->value_type == 0x03) { // BOOL
        return *reinterpret_cast<const bool*>(m_mapped_data + entry->value_offset);
    }
    return std::nullopt;
}

std::optional<std::string> YbinLoader::get_string(const std::string& section, const std::string& key)
{
    const YbinKeyValueEntry* entry = find_key_entry(section, key);
    if (entry && entry->value_type == 0x04) { // STRING
        // The value_offset for a string is the offset to the null-terminated string itself
        return std::string(m_mapped_data + entry->value_offset);
    }
    return std::nullopt;
}

} // namespace YINI
