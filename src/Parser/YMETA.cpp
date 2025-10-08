#include "YMETA.h"
#include <iostream>
#include <sstream>
#include <regex>

namespace yini
{

YMETA::YMETA()
    : version(YMETA_VERSION)
{
}

void YMETA::populateFromParser(const Parser& parser)
{
    sections = parser.getSections();
    defines = parser.getDefines();
    includes = parser.getIncludes();
    version = YMETA_VERSION;

    // Extract dynamic values and start their history
    for (const auto& [section_name, section] : sections)
    {
        for (const auto& [key, value] : section.entries)
        {
            if (value->isDynamic())
            {
                std::string full_key = section_name + "." + key;
                dynamic_values[full_key] = {value};
            }
        }
    }
}

bool YMETA::save(const std::string& output_file, uint32_t flags) const
{
    std::ofstream out(output_file, std::ios::binary);
    if (!out.is_open())
    {
        std::cerr << "Error: Could not open output file: " << output_file << std::endl;
        return false;
    }

    writeHeader(out, flags);

    if (flags & YMETA_CONTENT_FULL)
    {
        uint32_t include_count = static_cast<uint32_t>(includes.size());
        out.write(reinterpret_cast<const char*>(&include_count), sizeof(include_count));
        for (const auto& include : includes) writeString(out, include);

        uint32_t define_count = static_cast<uint32_t>(defines.size());
        out.write(reinterpret_cast<const char*>(&define_count), sizeof(define_count));
        for (const auto& [name, value] : defines)
        {
            writeString(out, name);
            writeValue(out, value);
        }

        uint32_t section_count = static_cast<uint32_t>(sections.size());
        out.write(reinterpret_cast<const char*>(&section_count), sizeof(section_count));
        for (const auto& [name, section] : sections)
        {
            writeString(out, name);
            writeSection(out, section);
        }
    }

    if (flags & YMETA_CONTENT_DYNAMIC_ONLY)
    {
        uint32_t dynamic_count = static_cast<uint32_t>(dynamic_values.size());
        out.write(reinterpret_cast<const char*>(&dynamic_count), sizeof(dynamic_count));
        for (const auto& [key, history] : dynamic_values)
        {
            writeString(out, key);
            uint32_t history_size = static_cast<uint32_t>(history.size());
            out.write(reinterpret_cast<const char*>(&history_size), sizeof(history_size));
            for (const auto& value : history)
            {
                writeValue(out, value);
            }
        }
    }

    out.close();
    return true;
}

bool YMETA::load(const std::string& input_file)
{
    std::ifstream in(input_file, std::ios::binary);
    if (!in.is_open()) return false;

    uint32_t flags;
    if (!readHeader(in, flags)) return false;

    if (flags & YMETA_CONTENT_FULL)
    {
        uint32_t include_count;
        in.read(reinterpret_cast<char*>(&include_count), sizeof(include_count));
        for (uint32_t i = 0; i < include_count; ++i) includes.push_back(readString(in));

        uint32_t define_count;
        in.read(reinterpret_cast<char*>(&define_count), sizeof(define_count));
        for (uint32_t i = 0; i < define_count; ++i)
        {
            std::string name = readString(in);
            defines[name] = readValue(in);
        }

        uint32_t section_count;
        in.read(reinterpret_cast<char*>(&section_count), sizeof(section_count));
        for (uint32_t i = 0; i < section_count; ++i)
        {
            std::string name = readString(in);
            sections[name] = readSection(in);
        }
    }

    if (flags & YMETA_CONTENT_DYNAMIC_ONLY)
    {
        uint32_t dynamic_count;
        in.read(reinterpret_cast<char*>(&dynamic_count), sizeof(dynamic_count));
        for (uint32_t i = 0; i < dynamic_count; ++i)
        {
            std::string key = readString(in);
            uint32_t history_size;
            in.read(reinterpret_cast<char*>(&history_size), sizeof(history_size));
            std::vector<std::shared_ptr<Value>> history;
            for (uint32_t j = 0; j < history_size; ++j)
            {
                history.push_back(readValue(in));
            }
            dynamic_values[key] = history;
        }
    }

    in.close();
    return true;
}

void YMETA::updateDynamicValue(const std::string& key, const std::shared_ptr<Value>& value)
{
    auto& history = dynamic_values[key];
    history.insert(history.begin(), value);
    if (history.size() > MAX_DYNAMIC_HISTORY)
    {
        history.pop_back();
    }
}

// Deprecated methods
bool YMETA::serialize(const Parser& parser, const std::string& output_file)
{
    populateFromParser(parser);
    return save(output_file, YMETA_CONTENT_FULL);
}

bool YMETA::deserialize(const std::string& input_file)
{
    return load(input_file);
}

std::string YMETA::toYINI() const
{
    std::ostringstream oss;
    
    if (!includes.empty())
    {
        oss << "[#include]" << std::endl;
        for (const auto& include : includes) oss << "+= \"" << include << "\"" << std::endl;
        oss << std::endl;
    }
    
    if (!defines.empty())
    {
        oss << "[#define]" << std::endl;
        for (const auto& [name, value] : defines) oss << name << " = " << value->toString() << std::endl;
        oss << std::endl;
    }
    
    for (const auto& [name, section] : sections)
    {
        oss << "[" << name << "]";
        if (!section.inherited_sections.empty())
        {
            oss << " : ";
            for (size_t i = 0; i < section.inherited_sections.size(); ++i)
            {
                if (i > 0) oss << ", ";
                oss << section.inherited_sections[i];
            }
        }
        oss << std::endl;
        
        for (const auto& [key, value] : section.entries)
        {
            oss << key << " = " << value->toString() << std::endl;
        }
        oss << std::endl;
    }
    
    return oss.str();
}

void YMETA::writeHeader(std::ofstream& out, uint32_t flags) const
{
    out.write(reinterpret_cast<const char*>(&YMETA_MAGIC), sizeof(YMETA_MAGIC));
    out.write(reinterpret_cast<const char*>(&version), sizeof(version));
    out.write(reinterpret_cast<const char*>(&flags), sizeof(flags));
}

void YMETA::writeString(std::ofstream& out, const std::string& str) const
{
    uint32_t length = static_cast<uint32_t>(str.length());
    out.write(reinterpret_cast<const char*>(&length), sizeof(length));
    out.write(str.c_str(), length);
}

void YMETA::writeValue(std::ofstream& out, const std::shared_ptr<Value>& value) const
{
    uint8_t type = static_cast<uint8_t>(value->getType());
    out.write(reinterpret_cast<const char*>(&type), sizeof(type));
    
    switch (value->getType())
    {
        case ValueType::INTEGER: {
            int64_t val = value->asInteger();
            out.write(reinterpret_cast<const char*>(&val), sizeof(val));
            break;
        }
        case ValueType::FLOAT: {
            double val = value->asFloat();
            out.write(reinterpret_cast<const char*>(&val), sizeof(val));
            break;
        }
        case ValueType::BOOLEAN: {
            bool val = value->asBoolean();
            out.write(reinterpret_cast<const char*>(&val), sizeof(val));
            break;
        }
        case ValueType::STRING: {
            writeString(out, value->asString());
            break;
        }
        case ValueType::ARRAY:
        case ValueType::LIST: {
            auto arr = value->asArray();
            uint32_t size = static_cast<uint32_t>(arr.size());
            out.write(reinterpret_cast<const char*>(&size), sizeof(size));
            for (const auto& elem : arr) writeValue(out, elem);
            break;
        }
        default:
            writeString(out, value->toString());
            break;
    }
}

void YMETA::writeSection(std::ofstream& out, const Section& section) const
{
    uint32_t inherit_count = static_cast<uint32_t>(section.inherited_sections.size());
    out.write(reinterpret_cast<const char*>(&inherit_count), sizeof(inherit_count));
    for (const auto& inherit : section.inherited_sections) writeString(out, inherit);
    
    uint32_t entry_count = static_cast<uint32_t>(section.entries.size());
    out.write(reinterpret_cast<const char*>(&entry_count), sizeof(entry_count));
    for (const auto& [key, value] : section.entries)
    {
        writeString(out, key);
        writeValue(out, value);
    }
}

bool YMETA::readHeader(std::ifstream& in, uint32_t& flags)
{
    uint32_t magic;
    in.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    if (magic != YMETA_MAGIC) return false;
    
    in.read(reinterpret_cast<char*>(&version), sizeof(version));
    if (version > YMETA_VERSION) return false;

    in.read(reinterpret_cast<char*>(&flags), sizeof(flags));
    
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
        case ValueType::INTEGER: {
            int64_t val;
            in.read(reinterpret_cast<char*>(&val), sizeof(val));
            return std::make_shared<Value>(val);
        }
        case ValueType::FLOAT: {
            double val;
            in.read(reinterpret_cast<char*>(&val), sizeof(val));
            return std::make_shared<Value>(val);
        }
        case ValueType::BOOLEAN: {
            bool val;
            in.read(reinterpret_cast<char*>(&val), sizeof(val));
            return std::make_shared<Value>(val);
        }
        case ValueType::STRING: {
            return std::make_shared<Value>(readString(in));
        }
        case ValueType::ARRAY:
        case ValueType::LIST: {
            uint32_t size;
            in.read(reinterpret_cast<char*>(&size), sizeof(size));
            Value::ArrayType arr;
            for (uint32_t i = 0; i < size; ++i) arr.push_back(readValue(in));
            return std::make_shared<Value>(arr);
        }
        default:
            return std::make_shared<Value>(readString(in));
    }
}

Section YMETA::readSection(std::ifstream& in)
{
    Section section;
    
    uint32_t inherit_count;
    in.read(reinterpret_cast<char*>(&inherit_count), sizeof(inherit_count));
    for (uint32_t i = 0; i < inherit_count; ++i)
    {
        section.inherited_sections.push_back(readString(in));
    }
    
    uint32_t entry_count;
    in.read(reinterpret_cast<char*>(&entry_count), sizeof(entry_count));
    for (uint32_t i = 0; i < entry_count; ++i)
    {
        std::string key = readString(in);
        section.entries[key] = readValue(in);
    }
    
    return section;
}

bool YMETA::mergeUpdatesIntoYiniFile(const std::string& yini_input_path, const std::string& yini_output_path) const
{
    std::ifstream in(yini_input_path);
    if (!in.is_open())
    {
        std::cerr << "Error: Could not open input file: " << yini_input_path << std::endl;
        return false;
    }

    std::ofstream out(yini_output_path);
    if (!out.is_open())
    {
        std::cerr << "Error: Could not open output file: " << yini_output_path << std::endl;
        return false;
    }

    std::string line;
    std::string current_section;
    std::regex section_regex(R"(\s*\[\s*([^\]\s]+)\s*\])");
    std::regex key_regex(R"(^\s*([a-zA-Z0-9_]+)\s*=)");

    while (std::getline(in, line))
    {
        std::smatch match;
        if (std::regex_search(line, match, section_regex))
        {
            current_section = match[1].str();
            out << line << std::endl;
        }
        else if (std::regex_search(line, match, key_regex))
        {
            std::string key = match[1].str();
            std::string full_key = current_section + "." + key;

            auto it = dynamic_values.find(full_key);
            if (it != dynamic_values.end() && !it->second.empty())
            {
                const auto& latest_value = it->second.front();
                size_t equals_pos = line.find('=');
                std::string line_start = line.substr(0, equals_pos);
                out << line_start << "= " << latest_value->toString() << std::endl;
            }
            else
            {
                out << line << std::endl;
            }
        }
        else
        {
            out << line << std::endl;
        }
    }

    in.close();
    out.close();
    return true;
}

} // namespace yini
