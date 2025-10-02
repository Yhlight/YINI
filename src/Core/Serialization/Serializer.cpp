#include "Serializer.h"
#include "Format.h"
#include "Core/YiniValue.h"
#include "Core/DynaValue.h"
#include <stdexcept>
#include <vector>
#include <map>
#include <variant>

namespace YINI
{
    namespace Serialization
    {
        // Helper visitor for serializing a YiniValue
        struct SerializeVisitor {
            std::ofstream& out;
            Serializer& serializer;

            void operator()(std::monostate) const {
                DataType tag = DataType::NIL;
                out.write(reinterpret_cast<const char*>(&tag), sizeof(tag));
            }
            void operator()(bool val) const {
                DataType tag = DataType::BOOL;
                out.write(reinterpret_cast<const char*>(&tag), sizeof(tag));
                out.write(reinterpret_cast<const char*>(&val), sizeof(val));
            }
            void operator()(double val) const {
                DataType tag = DataType::DOUBLE;
                out.write(reinterpret_cast<const char*>(&tag), sizeof(tag));
                out.write(reinterpret_cast<const char*>(&val), sizeof(val));
            }
            void operator()(const std::string& val) const {
                DataType tag = DataType::STRING;
                out.write(reinterpret_cast<const char*>(&tag), sizeof(tag));
                serializer.write_string(out, val);
            }
            void operator()(const std::unique_ptr<YiniArray>& val) const {
                DataType tag = DataType::VECTOR;
                out.write(reinterpret_cast<const char*>(&tag), sizeof(tag));
                size_t count = val->size();
                out.write(reinterpret_cast<const char*>(&count), sizeof(count));
                for (const auto& item : *val) {
                    serializer.write_value(out, item);
                }
            }
            void operator()(const std::unique_ptr<YiniMap>& val) const {
                DataType tag = DataType::MAP;
                out.write(reinterpret_cast<const char*>(&tag), sizeof(tag));
                size_t count = val->size();
                out.write(reinterpret_cast<const char*>(&count), sizeof(count));
                for (const auto& pair : *val) {
                    serializer.write_string(out, pair.first);
                    serializer.write_value(out, pair.second);
                }
            }
            void operator()(const std::unique_ptr<DynaValue>& val) const {
                // When serializing a DynaValue, we serialize its contained value.
                if (val) {
                    serializer.write_value(out, val->get());
                } else {
                    (*this)(std::monostate{});
                }
            }
        };

        void Serializer::serialize(const std::map<std::string, std::map<std::string, YiniValue>>& data, const std::string& filepath)
        {
            std::ofstream out(filepath, std::ios::binary);
            if (!out) {
                throw std::runtime_error("Cannot open file for writing: " + filepath);
            }

            size_t section_count = data.size();
            out.write(reinterpret_cast<const char*>(&section_count), sizeof(section_count));

            for (const auto& section_pair : data) {
                write_string(out, section_pair.first);

                size_t kv_count = section_pair.second.size();
                out.write(reinterpret_cast<const char*>(&kv_count), sizeof(kv_count));

                for (const auto& kv_pair : section_pair.second) {
                    write_string(out, kv_pair.first);
                    write_value(out, kv_pair.second);
                }
            }
        }

        void Serializer::write_string(std::ofstream& out, const std::string& str)
        {
            size_t len = str.length();
            out.write(reinterpret_cast<const char*>(&len), sizeof(len));
            out.write(str.c_str(), len);
        }

        void Serializer::write_value(std::ofstream& out, const YiniValue& value)
        {
            std::visit(SerializeVisitor{out, *this}, value.m_value);
        }
    }
}