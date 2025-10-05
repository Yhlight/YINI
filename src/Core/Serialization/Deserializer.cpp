#include "Deserializer.h"

#include <stdexcept>

#include "Core/YiniValue.h"
#include "Format.h"

namespace YINI {
namespace Serialization {
std::map<std::string, std::map<std::string, YiniValue, std::less<>>, std::less<>> Deserializer::deserialize(
    const std::string& filepath) {
    std::ifstream in(filepath, std::ios::binary);
    if (!in) {
        throw std::runtime_error("Cannot open file for reading: " + filepath);
    }

    std::map<std::string, std::map<std::string, YiniValue, std::less<>>, std::less<>> data;

    size_t section_count;
    in.read(reinterpret_cast<char*>(&section_count), sizeof(section_count));

    for (size_t i = 0; i < section_count; ++i) {
        std::string section_name = read_string(in);
        std::map<std::string, YiniValue, std::less<>> section_data;

        size_t kv_count;
        in.read(reinterpret_cast<char*>(&kv_count), sizeof(kv_count));

        for (size_t j = 0; j < kv_count; ++j) {
            std::string key = read_string(in);
            section_data[key] = read_value(in);
        }
        data[section_name] = section_data;
    }

    return data;
}

std::string Deserializer::read_string(std::ifstream& in) {
    size_t len;
    in.read(reinterpret_cast<char*>(&len), sizeof(len));
    std::string str(len, '\0');
    in.read(&str[0], len);
    return str;
}

YiniValue Deserializer::read_value(std::ifstream& in) {
    DataType tag;
    in.read(reinterpret_cast<char*>(&tag), sizeof(tag));

    switch (tag) {
        case DataType::NIL:
            return YiniValue{};
        case DataType::BOOL: {
            bool val;
            in.read(reinterpret_cast<char*>(&val), sizeof(val));
            return YiniValue(val);
        }
        case DataType::DOUBLE: {
            double val;
            in.read(reinterpret_cast<char*>(&val), sizeof(val));
            return YiniValue(val);
        }
        case DataType::STRING: {
            return YiniValue(read_string(in));
        }
        case DataType::VECTOR: {
            size_t count;
            in.read(reinterpret_cast<char*>(&count), sizeof(count));
            YiniArray vec;
            vec.reserve(count);
            for (size_t i = 0; i < count; ++i) {
                vec.push_back(read_value(in));
            }
            return YiniValue(std::move(vec));
        }
        case DataType::MAP: {
            size_t count;
            in.read(reinterpret_cast<char*>(&count), sizeof(count));
            YiniMap map;
            for (size_t i = 0; i < count; ++i) {
                std::string key = read_string(in);
                map[key] = read_value(in);
            }
            return YiniValue(std::move(map));
        }
    }
    throw std::runtime_error("Unknown data type tag in .ymeta file.");
}
}  // namespace Serialization
}  // namespace YINI