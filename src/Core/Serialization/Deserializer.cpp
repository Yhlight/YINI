#include "Deserializer.h"
#include "Format.h"
#include "Core/YiniException.h"
#include <stdexcept>

namespace YINI
{
    namespace Serialization
    {
        std::map<std::string, std::map<std::string, std::any>> Deserializer::deserialize(const std::string& filepath)
        {
            std::ifstream in(filepath, std::ios::binary);
            if (!in) {
                throw YiniException("Cannot open file for reading", filepath, 0, 0);
            }

            std::map<std::string, std::map<std::string, std::any>> data;

            size_t section_count;
            in.read(reinterpret_cast<char*>(&section_count), sizeof(section_count));

            for (size_t i = 0; i < section_count; ++i) {
                std::string section_name = read_string(in);
                std::map<std::string, std::any> section_data;

                size_t kv_count;
                in.read(reinterpret_cast<char*>(&kv_count), sizeof(kv_count));

                for (size_t j = 0; j < kv_count; ++j) {
                    std::string key = read_string(in);
                    section_data[key] = read_any(in, filepath);
                }
                data[section_name] = section_data;
            }

            return data;
        }

        std::string Deserializer::read_string(std::ifstream& in)
        {
            size_t len;
            in.read(reinterpret_cast<char*>(&len), sizeof(len));
            std::string str(len, '\0');
            in.read(&str[0], len);
            return str;
        }

        std::any Deserializer::read_any(std::ifstream& in, const std::string& filepath)
        {
            DataType tag;
            in.read(reinterpret_cast<char*>(&tag), sizeof(tag));

            switch (tag) {
                case DataType::NIL:
                    return std::any{};
                case DataType::BOOL: {
                    bool val;
                    in.read(reinterpret_cast<char*>(&val), sizeof(val));
                    return val;
                }
                case DataType::DOUBLE: {
                    double val;
                    in.read(reinterpret_cast<char*>(&val), sizeof(val));
                    return val;
                }
                case DataType::STRING: {
                    return read_string(in);
                }
                case DataType::VECTOR: {
                    size_t count;
                    in.read(reinterpret_cast<char*>(&count), sizeof(count));
                    std::vector<std::any> vec;
                    vec.reserve(count);
                    for (size_t i = 0; i < count; ++i) {
                        vec.push_back(read_any(in, filepath));
                    }
                    return vec;
                }
                case DataType::MAP: {
                    size_t count;
                    in.read(reinterpret_cast<char*>(&count), sizeof(count));
                    std::map<std::string, std::any> map;
                    for (size_t i = 0; i < count; ++i) {
                        std::string key = read_string(in);
                        map[key] = read_any(in, filepath);
                    }
                    return map;
                }
            }
            throw YiniException("Unknown data type tag in .ymeta file.", filepath, 0, 0);
        }
    }
}