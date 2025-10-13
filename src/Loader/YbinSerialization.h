#pragma once

#include "Utils/Endian.h"
#include "YbinFormat.h"
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

namespace YINI
{
namespace Ybin
{
// A helper class to read data sequentially from a memory buffer,
// ensuring correct endianness and handling potential overruns.
class BufferReader
{
  public:
    BufferReader(const char *data, size_t size) : m_data(data), m_size(size)
    {
    }

    template <typename T> T read()
    {
        if (m_offset + sizeof(T) > m_size)
        {
            throw std::runtime_error("Buffer overrun while reading from ybin data.");
        }
        T value;
        memcpy(&value, m_data + m_offset, sizeof(T));
        m_offset += sizeof(T);
        return value;
    }

    uint32_t read_u32_le()
    {
        return Utils::le32toh(read<uint32_t>());
    }

    uint64_t read_u64_le()
    {
        return Utils::le64toh(read<uint64_t>());
    }

    double read_double_le()
    {
        uint64_t as_int = read_u64_le();
        double val;
        memcpy(&val, &as_int, sizeof(double));
        return val;
    }

    // Reads a struct directly from the buffer and performs endian conversion on its members.
    // This is the core of the safe deserialization process.
    static void deserialize_header(FileHeader &header, const char *buffer, size_t size)
    {
        if (size < sizeof(FileHeader))
        {
            throw std::runtime_error("Invalid ybin file: header is too small.");
        }

        BufferReader reader(buffer, sizeof(FileHeader));
        header.magic = reader.read_u32_le();
        header.version = reader.read_u32_le();
        header.hash_table_offset = reader.read_u32_le();
        header.hash_table_size = reader.read_u32_le();
        header.entries_offset = reader.read_u32_le();
        header.entries_count = reader.read_u32_le();
        header.data_table_offset = reader.read_u32_le();
        header.data_table_compressed_size = reader.read_u32_le();
        header.data_table_uncompressed_size = reader.read_u32_le();
        header.string_table_offset = reader.read_u32_le();
        header.string_table_compressed_size = reader.read_u32_le();
        header.string_table_uncompressed_size = reader.read_u32_le();
    }

    static void deserialize_entry(HashTableEntry &entry, const char *buffer, size_t size)
    {
        if (size < sizeof(HashTableEntry))
        {
            throw std::runtime_error("Invalid ybin data: entry buffer is too small.");
        }
        BufferReader reader(buffer, sizeof(HashTableEntry));
        entry.key_hash = reader.read_u64_le();
        entry.key_offset = reader.read_u32_le();
        entry.value_type = reader.read<ValueType>();
        reader.read<uint8_t>(); // Skip 1 padding byte
        reader.read<uint8_t>(); // Skip 1 padding byte
        reader.read<uint8_t>(); // Skip 1 padding byte
        entry.value_offset = reader.read_u32_le();
        entry.next_entry_index = reader.read_u32_le();
    }

  private:
    const char *m_data;
    size_t m_size;
    size_t m_offset = 0;
};

// A helper class to write data sequentially to a memory buffer,
// ensuring correct endianness.
class BufferWriter
{
  public:
    BufferWriter(std::vector<char> &buffer) : m_buffer(buffer)
    {
    }

    template <typename T> void write(const T &value)
    {
        const char *bytes = reinterpret_cast<const char *>(&value);
        m_buffer.insert(m_buffer.end(), bytes, bytes + sizeof(T));
    }

    void write_u32_le(uint32_t value)
    {
        write(Utils::htole32(value));
    }

    void write_u64_le(uint64_t value)
    {
        write(Utils::htole64(value));
    }

    void write_double_le(double value)
    {
        uint64_t as_int;
        memcpy(&as_int, &value, sizeof(uint64_t));
        write_u64_le(as_int);
    }

    void write_bytes(const char *data, size_t size)
    {
        m_buffer.insert(m_buffer.end(), data, data + size);
    }

    size_t size() const
    {
        return m_buffer.size();
    }

  private:
    std::vector<char> &m_buffer;
};

} // namespace Ybin
} // namespace YINI
