#pragma once
#include <cstdint>

namespace YINI
{
namespace Ybin
{

// Define a magic number to identify .ybin files
constexpr uint32_t YBIN_MAGIC = 0x5942494E; // "YBIN"

// Enum to represent the different types of values we can store
enum class ValueType : uint8_t
{
    Null,
    Int64,
    Double,
    Bool,
    String, // The value_offset in YbinHashTableEntry will point into the string table
    ArrayInt,
    ArrayDouble,
    ArrayBool,
    ArrayString,
    Color,
    Coord
    // Note: Set, Map, List could be added here in the future
};

// File Header: The very first part of the .ybin file
#pragma pack(push, 1)
struct FileHeader
{
    uint32_t magic;           // Must be YBIN_MAGIC
    uint32_t version;         // Version number, e.g., 2
    uint32_t hash_table_offset; // Offset from the start of the file to the hash table buckets
    uint32_t hash_table_size;   // Number of buckets in the hash table
    uint32_t entries_offset;    // Offset to the array of hash table entries
    uint32_t entries_count;     // Total number of key-value entries
    uint32_t data_table_offset; // Offset to the start of the (potentially compressed) data table
    uint32_t data_table_compressed_size;
    uint32_t data_table_uncompressed_size;
    uint32_t string_table_offset; // Offset to the start of the (potentially compressed) string table
    uint32_t string_table_compressed_size;
    uint32_t string_table_uncompressed_size;
};
#pragma pack(pop)


// Hash Table Entry: Represents a single key-value pair.
// These are stored in an array and indexed by the hash table buckets.
#pragma pack(push, 1)
struct HashTableEntry
{
    uint64_t key_hash;          // 64-bit hash of the key string (e.g., "Section.key")
    uint32_t key_offset;        // Offset into the string table for the full key string
    ValueType value_type;       // The type of the value
    uint32_t value_offset;      // Offset to the value.
                                // For simple types (Int, Bool), this can be the value itself (type punning).
                                // For Double, Color, Coord, it's an offset into the data table.
                                // For String, it's an offset into the string table.
                                // For Arrays, it's an offset into the data table where the array data is stored.
    uint32_t next_entry_index;  // Index of the next entry in case of a hash collision (0 if none).
};
#pragma pack(pop)

// Structure for storing array metadata in the data table
#pragma pack(push, 1)
struct ArrayData
{
    uint32_t count; // Number of elements in the array
    // The actual array elements follow this struct immediately in the data table.
    // For ArrayString, the elements are uint32_t offsets into the string table.
};
#pragma pack(pop)

// Structures for Color and Coord in the data table
#pragma pack(push, 1)
struct ColorData
{
    uint8_t r, g, b;
};

struct CoordData
{
    double x, y, z;
};
#pragma pack(pop)


} // namespace Ybin
} // namespace YINI
