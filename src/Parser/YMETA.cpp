#include "YMETA.h"
#include <iostream>
#include <sstream>

namespace yini
{

bool YMETA::serialize(const Interpreter& interpreter, const std::string& output_file)
{
    std::ofstream out(output_file, std::ios::binary);
    if (!out.is_open())
    {
        std::cerr << "Error: Could not open output file: " << output_file << std::endl;
        return false;
    }
    
    // Copy data from interpreter
    sections = interpreter.getSections();
    defines = interpreter.getDefines();
    includes = interpreter.getIncludes();
    version = YMETA_VERSION;
    
    // Write header
    writeHeader(out);
    
    // Write includes
    uint32_t include_count = static_cast<uint32_t>(includes.size());
    out.write(reinterpret_cast<const char*>(&include_count), sizeof(include_count));
    for (const auto& include : includes)
    {
        writeString(out, include);
    }
    
    // Write defines
    uint32_t define_count = static_cast<uint32_t>(defines.size());
    out.write(reinterpret_cast<const char*>(&define_count), sizeof(define_count));
    for (const auto& [name, value] : defines)
    {
        writeString(out, name);
        writeValue(out, value);
    }
    
    // Write sections
    uint32_t section_count = static_cast<uint32_t>(sections.size());
    out.write(reinterpret_cast<const char*>(&section_count), sizeof(section_count));
    for (const auto& [name, section] : sections)
    {
        writeString(out, name);
        writeSection(out, section);
    }
    
    out.close();
    return true;
}

bool YMETA::deserialize(const std::string& input_file)
{
    std::ifstream in(input_file, std::ios::binary);
    if (!in.is_open())
    {
        std::cerr << "Error: Could not open input file: " << input_file << std::endl;
        return false;
    }
    
    // Read header
    if (!readHeader(in))
    {
        return false;
    }
    
    // Read includes
    uint32_t include_count;
    in.read(reinterpret_cast<char*>(&include_count), sizeof(include_count));
    includes.clear();
    for (uint32_t i = 0; i < include_count; i++)
    {
        includes.push_back(readString(in));
    }
    
    // Read defines
    uint32_t define_count;
    in.read(reinterpret_cast<char*>(&define_count), sizeof(define_count));
    defines.clear();
    for (uint32_t i = 0; i < define_count; i++)
    {
        std::string name = readString(in);
        auto value = readValue(in);
        defines[name] = value;
    }
    
    // Read sections
    uint32_t section_count;
    in.read(reinterpret_cast<char*>(&section_count), sizeof(section_count));
    sections.clear();
    for (uint32_t i = 0; i < section_count; i++)
    {
        std::string name = readString(in);
        Section section = readSection(in);
        sections[name] = section;
    }
    
    in.close();
    return true;
}

std::string YMETA::toYINI() const
{
    std::ostringstream oss;
    
    // Write includes
    if (!includes.empty())
    {
        oss << "[#include]" << std::endl;
        for (const auto& include : includes)
        {
            oss << "+= \"" << include << "\"" << std::endl;
        }
        oss << std::endl;
    }
    
    // Write defines
    if (!defines.empty())
    {
        oss << "[#define]" << std::endl;
        for (const auto& [name, value] : defines)
        {
            oss << name << " = " << value->toString() << std::endl;
        }
        oss << std::endl;
    }
    
    // Write sections
    for (const auto& [name, section] : sections)
    {
        oss << "[" << name << "]";
        
        // Write inheritance
        if (!section.inherited_sections.empty())
        {
            oss << " : ";
            for (size_t i = 0; i < section.inherited_sections.size(); i++)
            {
                if (i > 0) oss << ", ";
                oss << section.inherited_sections[i];
            }
        }
        
        oss << std::endl;
        
        // Write entries
        for (const auto& [key, value] : section.entries)
        {
            oss << key << " = " << value->toString() << std::endl;
        }
        
        oss << std::endl;
    }
    
    return oss.str();
}

void YMETA::writeHeader(std::ofstream& out)
{
    out.write(reinterpret_cast<const char*>(&YMETA_MAGIC), sizeof(YMETA_MAGIC));
    out.write(reinterpret_cast<const char*>(&version), sizeof(version));
}

void YMETA::writeString(std::ofstream& out, const std::string& str)
{
    uint32_t length = static_cast<uint32_t>(str.length());
    out.write(reinterpret_cast<const char*>(&length), sizeof(length));
    out.write(str.c_str(), length);
}

void YMETA::writeValue(std::ofstream& out, const std::shared_ptr<Value>& value)
{
    // Write type
    uint8_t type = static_cast<uint8_t>(value->getType());
    out.write(reinterpret_cast<const char*>(&type), sizeof(type));
    
    // Write value based on type
    switch (value->getType())
    {
        case ValueType::INTEGER:
        {
            int64_t val = value->asInteger();
            out.write(reinterpret_cast<const char*>(&val), sizeof(val));
            break;
        }
        case ValueType::FLOAT:
        {
            double val = value->asFloat();
            out.write(reinterpret_cast<const char*>(&val), sizeof(val));
            break;
        }
        case ValueType::BOOLEAN:
        {
            bool val = value->asBoolean();
            out.write(reinterpret_cast<const char*>(&val), sizeof(val));
            break;
        }
        case ValueType::STRING:
        {
            writeString(out, value->asString());
            break;
        }
        case ValueType::ARRAY:
        case ValueType::LIST:
        {
            auto arr = value->asArray();
            uint32_t size = static_cast<uint32_t>(arr.size());
            out.write(reinterpret_cast<const char*>(&size), sizeof(size));
            for (const auto& elem : arr)
            {
                writeValue(out, elem);
            }
            break;
        }
        default:
            // For now, serialize other types as strings
            writeString(out, value->toString());
            break;
    }
}

void YMETA::writeSection(std::ofstream& out, const Section& section)
{
    // Write inherited sections
    uint32_t inherit_count = static_cast<uint32_t>(section.inherited_sections.size());
    out.write(reinterpret_cast<const char*>(&inherit_count), sizeof(inherit_count));
    for (const auto& inherit : section.inherited_sections)
    {
        writeString(out, inherit);
    }
    
    // Write entries
    uint32_t entry_count = static_cast<uint32_t>(section.entries.size());
    out.write(reinterpret_cast<const char*>(&entry_count), sizeof(entry_count));
    for (const auto& [key, value] : section.entries)
    {
        writeString(out, key);
        writeValue(out, value);
    }
}

bool YMETA::readHeader(std::ifstream& in)
{
    uint32_t magic;
    in.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    
    if (magic != YMETA_MAGIC)
    {
        std::cerr << "Error: Invalid YMETA file (bad magic number)" << std::endl;
        return false;
    }
    
    in.read(reinterpret_cast<char*>(&version), sizeof(version));
    
    if (version > YMETA_VERSION)
    {
        std::cerr << "Error: YMETA file version " << version 
                  << " is newer than supported version " << YMETA_VERSION << std::endl;
        return false;
    }
    
    return true;
}

std::string YMETA::readString(std::ifstream& in)
{
    uint32_t length;
    in.read(reinterpret_cast<char*>(&length), sizeof(length));
    
    std::string str(length, '\0');
    in.read(&str[0], length);
    
    return str;
}

std::shared_ptr<Value> YMETA::readValue(std::ifstream& in)
{
    uint8_t type_byte;
    in.read(reinterpret_cast<char*>(&type_byte), sizeof(type_byte));
    
    ValueType type = static_cast<ValueType>(type_byte);
    
    switch (type)
    {
        case ValueType::INTEGER:
        {
            int64_t val;
            in.read(reinterpret_cast<char*>(&val), sizeof(val));
            return std::make_shared<Value>(val);
        }
        case ValueType::FLOAT:
        {
            double val;
            in.read(reinterpret_cast<char*>(&val), sizeof(val));
            return std::make_shared<Value>(val);
        }
        case ValueType::BOOLEAN:
        {
            bool val;
            in.read(reinterpret_cast<char*>(&val), sizeof(val));
            return std::make_shared<Value>(val);
        }
        case ValueType::STRING:
        {
            return std::make_shared<Value>(readString(in));
        }
        case ValueType::ARRAY:
        case ValueType::LIST:
        {
            uint32_t size;
            in.read(reinterpret_cast<char*>(&size), sizeof(size));
            
            Value::ArrayType arr;
            for (uint32_t i = 0; i < size; i++)
            {
                arr.push_back(readValue(in));
            }
            return std::make_shared<Value>(arr);
        }
        default:
            // Fallback: read as string
            return std::make_shared<Value>(readString(in));
    }
}

Section YMETA::readSection(std::ifstream& in)
{
    Section section;
    
    // Read inherited sections
    uint32_t inherit_count;
    in.read(reinterpret_cast<char*>(&inherit_count), sizeof(inherit_count));
    for (uint32_t i = 0; i < inherit_count; i++)
    {
        section.inherited_sections.push_back(readString(in));
    }
    
    // Read entries
    uint32_t entry_count;
    in.read(reinterpret_cast<char*>(&entry_count), sizeof(entry_count));
    for (uint32_t i = 0; i < entry_count; i++)
    {
        std::string key = readString(in);
        auto value = readValue(in);
        section.entries[key] = value;
    }
    
    return section;
}

} // namespace yini
