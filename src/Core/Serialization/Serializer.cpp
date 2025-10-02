#include "Serializer.h"
#include "Format.h"
#include "Core/YiniException.h"
#include <vector>
#include <map>

namespace YINI
{
    namespace Serialization
    {
        void Serializer::serialize(const std::map<std::string, std::map<std::string, std::any>>& data, const std::string& filepath)
        {
            std::ofstream out(filepath, std::ios::binary);
            if (!out) {
                throw YiniException("Cannot open file for writing", filepath, 0, 0);
            }

            size_t section_count = data.size();
            out.write(reinterpret_cast<const char*>(&section_count), sizeof(section_count));

            for (const auto& section_pair : data) {
                write_string(out, section_pair.first);

                size_t kv_count = section_pair.second.size();
                out.write(reinterpret_cast<const char*>(&kv_count), sizeof(kv_count));

                for (const auto& kv_pair : section_pair.second) {
                    write_string(out, kv_pair.first);
                    write_any(out, kv_pair.second);
                }
            }
        }

        void Serializer::write_string(std::ofstream& out, const std::string& str)
        {
            size_t len = str.length();
            out.write(reinterpret_cast<const char*>(&len), sizeof(len));
            out.write(str.c_str(), len);
        }

        void Serializer::write_any(std::ofstream& out, const std::any& value)
        {
            const auto& type = value.type();
            if (type == typeid(double)) {
                DataType tag = DataType::DOUBLE;
                out.write(reinterpret_cast<const char*>(&tag), sizeof(tag));
                double val = std::any_cast<double>(value);
                out.write(reinterpret_cast<const char*>(&val), sizeof(val));
            } else if (type == typeid(bool)) {
                DataType tag = DataType::BOOL;
                out.write(reinterpret_cast<const char*>(&tag), sizeof(tag));
                bool val = std::any_cast<bool>(value);
                out.write(reinterpret_cast<const char*>(&val), sizeof(val));
            } else if (type == typeid(std::string)) {
                DataType tag = DataType::STRING;
                out.write(reinterpret_cast<const char*>(&tag), sizeof(tag));
                write_string(out, std::any_cast<std::string>(value));
            } else if (type == typeid(std::vector<std::any>)) {
                DataType tag = DataType::VECTOR;
                out.write(reinterpret_cast<const char*>(&tag), sizeof(tag));
                const auto& vec = std::any_cast<const std::vector<std::any>&>(value);
                size_t count = vec.size();
                out.write(reinterpret_cast<const char*>(&count), sizeof(count));
                for (const auto& item : vec) {
                    write_any(out, item);
                }
            } else if (type == typeid(std::map<std::string, std::any>)) {
                DataType tag = DataType::MAP;
                out.write(reinterpret_cast<const char*>(&tag), sizeof(tag));
                const auto& map = std::any_cast<const std::map<std::string, std::any>&>(value);
                size_t count = map.size();
                out.write(reinterpret_cast<const char*>(&count), sizeof(count));
                for (const auto& pair : map) {
                    write_string(out, pair.first);
                    write_any(out, pair.second);
                }
            } else {
                // For nil or unsupported types
                DataType tag = DataType::NIL;
                out.write(reinterpret_cast<const char*>(&tag), sizeof(tag));
            }
        }
    }
}