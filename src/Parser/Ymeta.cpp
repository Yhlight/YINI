#include "Ymeta.h"
#include <fstream>
#include <stdexcept>
#include <variant>

namespace Ymeta
{
    // --- Binary I/O Helpers ---
    template<typename T>
    void write_binary(std::ofstream& stream, const T& value)
    {
        stream.write(reinterpret_cast<const char*>(&value), sizeof(T));
    }

    template<typename T>
    T read_binary(std::ifstream& stream)
    {
        T value;
        stream.read(reinterpret_cast<char*>(&value), sizeof(T));
        return value;
    }

    void write_string(std::ofstream& stream, const std::string& str)
    {
        uint32_t size = str.size();
        write_binary(stream, size);
        stream.write(str.c_str(), size);
    }

    std::string read_string(std::ifstream& stream)
    {
        uint32_t size = read_binary<uint32_t>(stream);
        std::string str(size, '\0');
        stream.read(&str[0], size);
        return str;
    }

    void serializeValue(std::ofstream& stream, const YiniValue& yiniValue);
    YiniValue deserializeValue(std::ifstream& stream);

    // --- Serializer Implementation ---
    void Serializer::serialize(const YiniFile& ast, const std::string& filepath)
    {
        std::ofstream stream(filepath, std::ios::binary);
        if (!stream)
        {
            throw std::runtime_error("Cannot open file for writing: " + filepath);
        }

        stream.write("YMET", 4);
        write_binary<uint8_t>(stream, 1);

        write_binary<uint8_t>(stream, static_cast<uint8_t>(MetaTag::YINI_FILE_START));

        write_binary<uint8_t>(stream, static_cast<uint8_t>(MetaTag::DEFINES_START));
        write_binary<uint32_t>(stream, ast.definesMap.size());
        for(const auto& [key, value] : ast.definesMap)
        {
            write_string(stream, key);
            serializeValue(stream, value);
        }

        write_binary<uint8_t>(stream, static_cast<uint8_t>(MetaTag::INCLUDES_START));
        write_binary<uint32_t>(stream, ast.includePaths.size());
        for(const auto& include : ast.includePaths)
        {
            write_string(stream, include);
        }

        write_binary<uint8_t>(stream, static_cast<uint8_t>(MetaTag::SECTIONS_START));
        write_binary<uint32_t>(stream, ast.sectionsMap.size());
        for(const auto& [name, section] : ast.sectionsMap)
        {
            write_binary<uint8_t>(stream, static_cast<uint8_t>(MetaTag::SECTION_START));
            write_string(stream, name);

            write_binary<uint8_t>(stream, static_cast<uint8_t>(MetaTag::INHERITS_START));
            write_binary<uint32_t>(stream, section.inherits.size());
            for(const auto& inherit : section.inherits)
            {
                write_string(stream, inherit);
            }

            write_binary<uint8_t>(stream, static_cast<uint8_t>(MetaTag::KEY_VALUES_START));
            write_binary<uint32_t>(stream, section.keyValues.size());
            for(const auto& [key, value] : section.keyValues)
            {
                write_string(stream, key);
                serializeValue(stream, value);
            }

            write_binary<uint8_t>(stream, static_cast<uint8_t>(MetaTag::AUTO_INDEXED_START));
            write_binary<uint32_t>(stream, section.autoIndexedValues.size());
            for(const auto& value : section.autoIndexedValues)
            {
                serializeValue(stream, value);
            }
        }

        write_binary<uint8_t>(stream, static_cast<uint8_t>(MetaTag::YINI_FILE_END));
    }

    void serializeValue(std::ofstream& stream, const YiniValue& yiniValue)
    {
        std::visit([&stream](auto&& arg)
        {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::string>)
            {
                write_binary<uint8_t>(stream, static_cast<uint8_t>(MetaTag::STRING));
                write_string(stream, arg);
            }
            else if constexpr (std::is_same_v<T, int64_t>)
            {
                write_binary<uint8_t>(stream, static_cast<uint8_t>(MetaTag::INT64));
                write_binary(stream, arg);
            }
            else if constexpr (std::is_same_v<T, double>)
            {
                write_binary<uint8_t>(stream, static_cast<uint8_t>(MetaTag::DOUBLE));
                write_binary(stream, arg);
            }
            else if constexpr (std::is_same_v<T, bool>)
            {
                auto tag = arg ? MetaTag::BOOL_TRUE : MetaTag::BOOL_FALSE;
                write_binary<uint8_t>(stream, static_cast<uint8_t>(tag));
            }
            else if constexpr (std::is_same_v<T, YiniArray>)
            {
                write_binary<uint8_t>(stream, static_cast<uint8_t>(MetaTag::ARRAY));
                write_binary<uint32_t>(stream, arg.size());
                for(const auto& v : arg)
                {
                    serializeValue(stream, v);
                }
            }
            else if constexpr (std::is_same_v<T, YiniCoord>)
            {
                write_binary<uint8_t>(stream, static_cast<uint8_t>(MetaTag::COORD));
                write_binary(stream, arg);
            }
            else if constexpr (std::is_same_v<T, YiniMacroRef>)
            {
                throw std::runtime_error("Cannot serialize an unresolved macro reference: @" + arg.name);
            }
        }, yiniValue.value);
    }

    // --- Deserializer Implementation ---
    YiniFile Deserializer::deserialize(const std::string& filepath)
    {
        std::ifstream stream(filepath, std::ios::binary);
        if (!stream)
        {
            throw std::runtime_error("Cannot open file for reading: " + filepath);
        }

        char magic[4];
        stream.read(magic, 4);
        if (std::string(magic, 4) != "YMET")
        {
            throw std::runtime_error("Not a valid .ymeta file (bad magic number).");
        }
        uint8_t version = read_binary<uint8_t>(stream);
        if (version != 1)
        {
            throw std::runtime_error("Unsupported .ymeta version.");
        }

        YiniFile ast;
        if (read_binary<uint8_t>(stream) != static_cast<uint8_t>(MetaTag::YINI_FILE_START))
            throw std::runtime_error("YMETA file format error.");

        if (read_binary<uint8_t>(stream) != static_cast<uint8_t>(MetaTag::DEFINES_START)) throw std::runtime_error("YMETA format error.");
        uint32_t defines_count = read_binary<uint32_t>(stream);
        for (uint32_t i = 0; i < defines_count; ++i)
        {
            std::string key = read_string(stream);
            ast.definesMap[key] = deserializeValue(stream);
        }

        if (read_binary<uint8_t>(stream) != static_cast<uint8_t>(MetaTag::INCLUDES_START)) throw std::runtime_error("YMETA format error.");
        uint32_t includes_count = read_binary<uint32_t>(stream);
        for (uint32_t i = 0; i < includes_count; ++i)
        {
            ast.includePaths.push_back(read_string(stream));
        }

        if (read_binary<uint8_t>(stream) != static_cast<uint8_t>(MetaTag::SECTIONS_START)) throw std::runtime_error("YMETA format error.");
        uint32_t sections_count = read_binary<uint32_t>(stream);
        for(uint32_t i = 0; i < sections_count; ++i)
        {
            if (read_binary<uint8_t>(stream) != static_cast<uint8_t>(MetaTag::SECTION_START)) throw std::runtime_error("YMETA format error.");
            YiniSection section;
            section.name = read_string(stream);

            if (read_binary<uint8_t>(stream) != static_cast<uint8_t>(MetaTag::INHERITS_START)) throw std::runtime_error("YMETA format error.");
            uint32_t inherits_count = read_binary<uint32_t>(stream);
            for(uint32_t j=0; j < inherits_count; ++j) section.inherits.push_back(read_string(stream));

            if (read_binary<uint8_t>(stream) != static_cast<uint8_t>(MetaTag::KEY_VALUES_START)) throw std::runtime_error("YMETA format error.");
            uint32_t kv_count = read_binary<uint32_t>(stream);
            for(uint32_t j=0; j < kv_count; ++j)
            {
                std::string key = read_string(stream);
                section.keyValues[key] = deserializeValue(stream);
            }

            if (read_binary<uint8_t>(stream) != static_cast<uint8_t>(MetaTag::AUTO_INDEXED_START)) throw std::runtime_error("YMETA format error.");
            uint32_t auto_count = read_binary<uint32_t>(stream);
            for(uint32_t j=0; j < auto_count; ++j) section.autoIndexedValues.push_back(deserializeValue(stream));

            ast.sectionsMap[section.name] = section;
        }

        if (read_binary<uint8_t>(stream) != static_cast<uint8_t>(MetaTag::YINI_FILE_END))
            throw std::runtime_error("YMETA file format error: missing EOF tag.");

        return ast;
    }

    YiniValue deserializeValue(std::ifstream& stream)
    {
        MetaTag tag = static_cast<MetaTag>(read_binary<uint8_t>(stream));
        switch (tag)
        {
            case MetaTag::STRING: return YiniValue{read_string(stream)};
            case MetaTag::INT64: return YiniValue{read_binary<int64_t>(stream)};
            case MetaTag::DOUBLE: return YiniValue{read_binary<double>(stream)};
            case MetaTag::BOOL_TRUE: return YiniValue{true};
            case MetaTag::BOOL_FALSE: return YiniValue{false};
            case MetaTag::ARRAY:
            {
                uint32_t size = read_binary<uint32_t>(stream);
                YiniArray arr;
                for(uint32_t i = 0; i < size; ++i) arr.push_back(deserializeValue(stream));
                return YiniValue{arr};
            }
            case MetaTag::COORD: return YiniValue{read_binary<YiniCoord>(stream)};
            default: throw std::runtime_error("Unknown or unsupported tag in YMETA file.");
        }
    }
}
