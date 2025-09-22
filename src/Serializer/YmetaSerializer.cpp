#include "YmetaSerializer.h"
#include <fstream>
#include <vector>
#include <variant>

namespace Yini
{
    namespace
    {
        // Helper functions for binary serialization
        template<typename T>
        void write(std::ofstream& stream, const T& value)
        {
            stream.write(reinterpret_cast<const char*>(&value), sizeof(T));
        }

        void write(std::ofstream& stream, const std::string& str)
        {
            size_t len = str.length();
            write(stream, len);
            stream.write(str.c_str(), len);
        }

        void writeValue(std::ofstream& stream, const YiniValue& value);

        void writeVariant(std::ofstream& stream, const YiniVariant& variant)
        {
            uint8_t index = variant.index();
            write(stream, index);

            std::visit([&stream](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, YiniInteger> || std::is_same_v<T, YiniFloat> || std::is_same_v<T, YiniBoolean> || std::is_same_v<T, Coordinate2D> || std::is_same_v<T, Coordinate3D> || std::is_same_v<T, ColorRGB> || std::is_same_v<T, ColorRGBA>)
                {
                    write(stream, arg);
                }
                else if constexpr (std::is_same_v<T, YiniString>)
                {
                    write(stream, arg);
                }
                else if constexpr (std::is_same_v<T, YiniArray>)
                {
                    write(stream, arg.size());
                    for(const auto& v : arg) { writeValue(stream, v); }
                }
                else if constexpr (std::is_same_v<T, YiniMap>)
                {
                    write(stream, arg.size());
                    for(const auto& [k, v] : arg) { write(stream, k); writeValue(stream, v); }
                }
            }, variant);
        }

        void writeValue(std::ofstream& stream, const YiniValue& value)
        {
            writeVariant(stream, value.getVariant());
        }

        // Helper functions for binary deserialization
        template<typename T>
        void read(std::ifstream& stream, T& value)
        {
            stream.read(reinterpret_cast<char*>(&value), sizeof(T));
        }

        void read(std::ifstream& stream, std::string& str)
        {
            size_t len;
            read(stream, len);
            str.resize(len);
            stream.read(&str[0], len);
        }

        YiniValue readValue(std::ifstream& stream);

        template<size_t I = 0, typename... Tp>
        YiniVariant createVariant(size_t index, std::ifstream& stream)
        {
            if constexpr (I == sizeof...(Tp))
            {
                throw std::runtime_error("Bad variant index");
            }
            else
            {
                if (index == I)
                {
                    using T = std::tuple_element_t<I, std::tuple<Tp...>>;
                    if constexpr (std::is_same_v<T, YiniInteger> || std::is_same_v<T, YiniFloat> || std::is_same_v<T, YiniBoolean> || std::is_same_v<T, Coordinate2D> || std::is_same_v<T, Coordinate3D> || std::is_same_v<T, ColorRGB> || std::is_same_v<T, ColorRGBA>)
                    {
                        T val;
                        read(stream, val);
                        return YiniVariant(val);
                    }
                    else if constexpr (std::is_same_v<T, YiniString>)
                    {
                        T val;
                        read(stream, val);
                        return YiniVariant(val);
                    }
                    else if constexpr (std::is_same_v<T, YiniArray>)
                    {
                        size_t size;
                        read(stream, size);
                        T arr;
                        for(size_t i = 0; i < size; ++i) arr.push_back(readValue(stream));
                        return YiniVariant(arr);
                    }
                    else if constexpr (std::is_same_v<T, YiniMap>)
                    {
                        size_t size;
                        read(stream, size);
                        T map;
                        for(size_t i = 0; i < size; ++i) {
                            std::string key;
                            read(stream, key);
                            map[key] = readValue(stream);
                        }
                        return YiniVariant(map);
                    }
                }
                return createVariant<I + 1, Tp...>(index, stream);
            }
        }

        YiniValue readValue(std::ifstream& stream)
        {
            uint8_t index;
            read(stream, index);
            return YiniValue(createVariant<0, YiniInteger, YiniFloat, YiniBoolean, YiniString, Coordinate2D, Coordinate3D, ColorRGB, ColorRGBA, YiniArray, YiniMap>(index, stream));
        }

    }


    YmetaSerializer::YmetaSerializer() {}

    bool YmetaSerializer::save(const YiniData& data, const std::string& filepath)
    {
        std::ofstream file(filepath, std::ios::binary);
        if(!file.is_open()) return false;

        file.write("YMET", 4);
        uint32_t version = 1;
        write(file, version);

        write(file, data.getMacros().size());
        for(const auto& [key, value] : data.getMacros()) { write(file, key); writeValue(file, value); }

        write(file, data.getSections().size());
        for(const auto& [sectionName, section] : data.getSections())
        {
            write(file, sectionName);

            write(file, section.getKeyValues().size());
            for(const auto& [key, value] : section.getKeyValues()) { write(file, key); writeValue(file, value); }

            write(file, section.getValues().size());
            for(const auto& value : section.getValues()) { writeValue(file, value); }

            write(file, section.getInheritance().size());
            for(const auto& parent : section.getInheritance()) { write(file, parent); }
        }

        return true;
    }

    YiniData YmetaSerializer::load(const std::string& filepath)
    {
        std::ifstream file(filepath, std::ios::binary);
        if(!file.is_open()) return {};

        char magic[4];
        file.read(magic, 4);
        if(std::string(magic, 4) != "YMET") return {};

        uint32_t version;
        read(file, version);
        if(version != 1) return {};

        YiniData data;

        size_t macros_size;
        read(file, macros_size);
        for(size_t i=0; i<macros_size; ++i) { std::string key; read(file, key); data.addMacro(key, readValue(file)); }

        size_t sections_size;
        read(file, sections_size);
        for(size_t i=0; i<sections_size; ++i)
        {
            std::string sectionName;
            read(file, sectionName);
            YiniSection section(sectionName);

            size_t kvs_size;
            read(file, kvs_size);
            for(size_t j=0; j<kvs_size; ++j) { std::string key; read(file, key); section.addKeyValuePair(key, readValue(file)); }

            size_t values_size;
            read(file, values_size);
            for(size_t j=0; j<values_size; ++j) { section.addValue(readValue(file)); }

            size_t inheritance_size;
            read(file, inheritance_size);
            for(size_t j=0; j<inheritance_size; ++j) { std::string parent; read(file, parent); section.addInheritance(parent); }

            data.addSection(section);
        }

        return data;
    }
}
