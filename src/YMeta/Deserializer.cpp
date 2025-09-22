#include "Deserializer.h"
#include "YMeta.h"
#include <stdexcept>
#include <vector>

namespace YINI
{
    Deserializer::Deserializer(std::istream& in)
        : m_in(in)
    {
    }

    std::unique_ptr<YiniFile> Deserializer::deserialize()
    {
        // Read and verify header
        char magic[4];
        read(magic, sizeof(magic));
        if (std::string(magic, 4) != std::string(YMeta::MAGIC, 4))
        {
            throw std::runtime_error("Invalid YMeta file: bad magic number.");
        }

        uint8_t version;
        read(reinterpret_cast<char*>(&version), sizeof(version));
        if (version != YMeta::VERSION)
        {
            throw std::runtime_error("Unsupported YMeta version.");
        }

        auto yini_file = std::make_unique<YiniFile>();

        while (true)
        {
            YMeta::Tag tag = readTag();
            if (tag == YMeta::Tag::EndOfFile)
            {
                break;
            }

            if (tag == YMeta::Tag::SectionStart)
            {
                Section section;
                section.name = readString();

                uint32_t num_pairs;
                read(reinterpret_cast<char*>(&num_pairs), sizeof(num_pairs));

                for (uint32_t i = 0; i < num_pairs; ++i)
                {
                    YMeta::Tag kvp_tag = readTag();
                    if (kvp_tag != YMeta::Tag::KeyValuePair)
                    {
                        throw std::runtime_error("Malformed YMeta file: expected KeyValuePair tag.");
                    }
                    KeyValuePair kvp;
                    kvp.key = readString();
                    kvp.value = readValue();
                    section.pairs.push_back(std::move(kvp));
                }
                yini_file->sections[section.name] = std::move(section);
            }
            else
            {
                throw std::runtime_error("Malformed YMeta file: expected SectionStart tag.");
            }
        }

        return yini_file;
    }

    void Deserializer::read(char* data, std::size_t size)
    {
        m_in.read(data, size);
        if (!m_in)
        {
            throw std::runtime_error("Failed to read from YMeta stream.");
        }
    }

    YMeta::Tag Deserializer::readTag()
    {
        YMeta::Tag tag;
        read(reinterpret_cast<char*>(&tag), sizeof(tag));
        return tag;
    }

    std::string Deserializer::readString()
    {
        uint32_t len;
        read(reinterpret_cast<char*>(&len), sizeof(len));
        std::vector<char> buffer(len);
        read(buffer.data(), len);
        return std::string(buffer.begin(), buffer.end());
    }

    std::unique_ptr<Value> Deserializer::readValue()
    {
        auto value = std::make_unique<Value>();
        YMeta::Tag tag = readTag();

        switch (tag)
        {
            case YMeta::Tag::String:
                value->data = readString();
                break;
            case YMeta::Tag::Integer:
            {
                Integer i;
                read(reinterpret_cast<char*>(&i), sizeof(i));
                value->data = i;
                break;
            }
            case YMeta::Tag::Float:
            {
                Float f;
                read(reinterpret_cast<char*>(&f), sizeof(f));
                value->data = f;
                break;
            }
            case YMeta::Tag::Boolean:
            {
                Boolean b;
                read(reinterpret_cast<char*>(&b), sizeof(b));
                value->data = b;
                break;
            }
            case YMeta::Tag::Coordinate:
            {
                Coordinate c;
                read(reinterpret_cast<char*>(&c), sizeof(c));
                value->data = c;
                break;
            }
            case YMeta::Tag::Color:
            {
                Color c;
                read(reinterpret_cast<char*>(&c), sizeof(c));
                value->data = c;
                break;
            }
            case YMeta::Tag::ArrayStart:
            {
                uint32_t count;
                read(reinterpret_cast<char*>(&count), sizeof(count));
                Array array;
                for (uint32_t i = 0; i < count; ++i)
                {
                    array.push_back(readValue());
                }
                if (readTag() != YMeta::Tag::ArrayEnd)
                {
                    throw std::runtime_error("Malformed YMeta: missing ArrayEnd tag.");
                }
                value->data = std::move(array);
                break;
            }
            case YMeta::Tag::MapStart:
            {
                uint32_t count;
                read(reinterpret_cast<char*>(&count), sizeof(count));
                Map map;
                for (uint32_t i = 0; i < count; ++i)
                {
                    std::string key = readString();
                    map[key] = readValue();
                }
                if (readTag() != YMeta::Tag::MapEnd)
                {
                    throw std::runtime_error("Malformed YMeta: missing MapEnd tag.");
                }
                value->data = std::move(map);
                break;
            }
            default:
                throw std::runtime_error("Invalid tag read from YMeta stream.");
        }
        return value;
    }
}
