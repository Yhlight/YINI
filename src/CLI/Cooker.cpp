#include "Cooker.h"
#include <fstream>
#include <vector>
#include <iostream>
#include <cstdint>

namespace YINI
{

// A helper struct to hold intermediate data during cooking.
struct CookedValue
{
    uint8_t type;
    uint32_t data_offset;
};

Cooker::Cooker() {}

void Cooker::cook(const std::map<std::string, std::any>& config, const std::string& output_path)
{
    std::ofstream out_file(output_path, std::ios::binary);
    if (!out_file)
    {
        throw std::runtime_error("Cannot open output file: " + output_path);
    }

    // --- Pass 1: Flatten the data and build the string pool ---
    std::map<std::string, uint32_t> string_pool_offsets;
    std::vector<char> string_pool;
    auto add_to_string_pool = [&](const std::string& s) -> uint32_t {
        if (string_pool_offsets.count(s))
        {
            return string_pool_offsets[s];
        }
        uint32_t offset = string_pool.size();
        string_pool_offsets[s] = offset;
        string_pool.insert(string_pool.end(), s.begin(), s.end());
        string_pool.push_back('\0');
        return offset;
    };

    std::vector<char> data_block;
    std::map<std::string, std::map<std::string, CookedValue>> cooked_sections;

    // --- Pass 1: Re-structure the flat map from the resolver and build pools ---
    for (const auto& pair : config)
    {
        const std::string& full_key = pair.first;
        const std::any& value = pair.second;

        size_t separator_pos = full_key.find('.');
        if (separator_pos == std::string::npos) continue; // Skip keys without a section

        std::string section_name = full_key.substr(0, separator_pos);
        std::string key_name = full_key.substr(separator_pos + 1);

        add_to_string_pool(section_name);
        add_to_string_pool(key_name);

        CookedValue cooked_value;
        if (value.type() == typeid(double))
            {
                double double_val = std::any_cast<double>(value);
                // Check if it can be represented as an integer without data loss
                if (double_val == static_cast<double>(static_cast<int>(double_val)))
                {
                    cooked_value.type = 0x01; // INT
                    int int_val = static_cast<int>(double_val);
                    const char* bytes = reinterpret_cast<const char*>(&int_val);
                    cooked_value.data_offset = data_block.size();
                    data_block.insert(data_block.end(), bytes, bytes + sizeof(int));
                }
                else
                {
                    cooked_value.type = 0x02; // FLOAT
                    const char* bytes = reinterpret_cast<const char*>(&double_val);
                    cooked_value.data_offset = data_block.size();
                    data_block.insert(data_block.end(), bytes, bytes + sizeof(double));
                }
            }
            else if (value.type() == typeid(bool))
            {
                cooked_value.type = 0x03; // BOOL
                bool val = std::any_cast<bool>(value);
                cooked_value.data_offset = data_block.size();
                data_block.push_back(static_cast<char>(val));
            }
            else if (value.type() == typeid(std::string))
            {
                cooked_value.type = 0x04; // STRING
                std::string val = std::any_cast<std::string>(value);
                cooked_value.data_offset = add_to_string_pool(val);
            }
            else
            {
                // For simplicity, we'll skip complex types in this initial implementation.
                continue;
            }
            cooked_sections[section_name][key_name] = cooked_value;
    }

    // --- Pass 2: Assemble the final file ---

    // Calculate offsets
    uint32_t header_size = 16;
    uint32_t section_table_offset = header_size;
    uint32_t section_table_size = cooked_sections.size() * 12;
    uint32_t kv_table_offset = section_table_offset + section_table_size;

    // We need to calculate the total size of the key-value table first
    uint32_t total_kv_pairs = 0;
    for (const auto& section : cooked_sections)
    {
        total_kv_pairs += section.second.size();
    }
    uint32_t kv_table_size = total_kv_pairs * 9;

    uint32_t string_pool_offset = kv_table_offset + kv_table_size;
    uint32_t data_block_offset = string_pool_offset + string_pool.size();

    // -- Write Header --
    out_file.write("YINI", 4); // Magic number
    uint32_t version = 1;
    out_file.write(reinterpret_cast<const char*>(&version), sizeof(version));
    out_file.write(reinterpret_cast<const char*>(&section_table_offset), sizeof(section_table_offset));
    uint32_t num_sections = cooked_sections.size();
    out_file.write(reinterpret_cast<const char*>(&num_sections), sizeof(num_sections));

    // -- Write Section Table and Key-Value Table --
    uint32_t current_kv_offset = kv_table_offset;
    for (const auto& section : cooked_sections)
    {
        // Section Entry
        uint32_t name_offset = string_pool_offsets[section.first] + string_pool_offset;
        uint32_t kv_count = section.second.size();
        out_file.seekp(section_table_offset);
        out_file.write(reinterpret_cast<const char*>(&name_offset), sizeof(name_offset));
        out_file.write(reinterpret_cast<const char*>(&current_kv_offset), sizeof(current_kv_offset));
        out_file.write(reinterpret_cast<const char*>(&kv_count), sizeof(kv_count));
        section_table_offset += 12;

        // Key-Value Entries
        out_file.seekp(current_kv_offset);
        for (const auto& kv_pair : section.second)
        {
            uint32_t key_offset = string_pool_offsets[kv_pair.first] + string_pool_offset;
            uint8_t value_type = kv_pair.second.type;
            uint32_t value_offset = kv_pair.second.data_offset;

            if (value_type == 0x04) // STRING
            {
                 // The offset is already relative to the string pool start
                 value_offset += string_pool_offset;
            }
            else // Other types are relative to the data block start
            {
                 value_offset += data_block_offset;
            }

            out_file.write(reinterpret_cast<const char*>(&key_offset), sizeof(key_offset));
            out_file.write(reinterpret_cast<const char*>(&value_type), sizeof(value_type));
            out_file.write(reinterpret_cast<const char*>(&value_offset), sizeof(value_offset));
            current_kv_offset += 9;
        }
    }

    // -- Write String Pool --
    out_file.seekp(string_pool_offset);
    out_file.write(string_pool.data(), string_pool.size());

    // -- Write Data Block --
    out_file.seekp(data_block_offset);
    out_file.write(data_block.data(), data_block.size());

    std::cout << "Successfully cooked " << output_path << std::endl;
}

} // namespace YINI
