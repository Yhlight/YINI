#include "Serializer.h"
#include "YMeta.h"
#include <stdexcept>

namespace YINI
{
    Serializer::Serializer(const YiniFile& yini_file)
        : m_yini_file(yini_file), m_out(nullptr)
    {
    }

    void Serializer::serialize(std::ostream& out)
    {
        m_out = &out;

        // Write header
        write(YMeta::MAGIC, sizeof(YMeta::MAGIC));
        writeTag(static_cast<YMeta::Tag>(YMeta::VERSION));

        // Write sections
        for (const auto& [name, section] : m_yini_file.sections)
        {
            if (section.is_define_section || section.is_include_section) continue;

            writeTag(YMeta::Tag::SectionStart);
            writeString(name);

            uint32_t num_pairs = section.pairs.size();
            write(reinterpret_cast<const char*>(&num_pairs), sizeof(num_pairs));

            for (const auto& kvp : section.pairs)
            {
                writeTag(YMeta::Tag::KeyValuePair);
                writeString(kvp.key);
                writeValue(*kvp.value);
            }
        }
        writeTag(YMeta::Tag::EndOfFile);
    }

    void Serializer::write(const char* data, std::size_t size)
    {
        m_out->write(data, size);
    }

    void Serializer::writeTag(YMeta::Tag tag)
    {
        write(reinterpret_cast<const char*>(&tag), sizeof(tag));
    }

    void Serializer::writeString(const std::string& str)
    {
        uint32_t len = str.length();
        write(reinterpret_cast<const char*>(&len), sizeof(len));
        write(str.c_str(), len);
    }

    void Serializer::writeValue(const Value& value)
    {
        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, String>)
            {
                writeTag(YMeta::Tag::String);
                writeString(arg);
            }
            else if constexpr (std::is_same_v<T, Integer>)
            {
                writeTag(YMeta::Tag::Integer);
                write(reinterpret_cast<const char*>(&arg), sizeof(arg));
            }
            else if constexpr (std::is_same_v<T, Float>)
            {
                writeTag(YMeta::Tag::Float);
                write(reinterpret_cast<const char*>(&arg), sizeof(arg));
            }
            else if constexpr (std::is_same_v<T, Boolean>)
            {
                writeTag(YMeta::Tag::Boolean);
                write(reinterpret_cast<const char*>(&arg), sizeof(arg));
            }
            else if constexpr (std::is_same_v<T, Coordinate>)
            {
                writeTag(YMeta::Tag::Coordinate);
                write(reinterpret_cast<const char*>(&arg), sizeof(arg));
            }
            else if constexpr (std::is_same_v<T, Color>)
            {
                writeTag(YMeta::Tag::Color);
                write(reinterpret_cast<const char*>(&arg), sizeof(arg));
            }
            else if constexpr (std::is_same_v<T, Array>)
            {
                writeTag(YMeta::Tag::ArrayStart);
                uint32_t count = arg.size();
                write(reinterpret_cast<const char*>(&count), sizeof(count));
                for(const auto& item : arg)
                {
                    writeValue(*item);
                }
                writeTag(YMeta::Tag::ArrayEnd);
            }
            else if constexpr (std::is_same_v<T, Map>)
            {
                writeTag(YMeta::Tag::MapStart);
                uint32_t count = arg.size();
                write(reinterpret_cast<const char*>(&count), sizeof(count));
                for(const auto& [key, val] : arg)
                {
                    writeString(key);
                    writeValue(*val);
                }
                writeTag(YMeta::Tag::MapEnd);
            }
            else if constexpr (std::is_same_v<T, Macro>)
            {
                // Macros should be resolved before serialization
                throw std::runtime_error("Cannot serialize unresolved macro.");
            }
        }, value.data);
    }
}
