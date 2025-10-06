#ifndef YINI_YMETA_H
#define YINI_YMETA_H

#include "Parser.h"
#include <string>
#include <vector>
#include <fstream>
#include <cstdint>

namespace yini
{

// YMETA file format version
constexpr uint32_t YMETA_VERSION = 1;
constexpr uint32_t YMETA_MAGIC = 0x59494E49; // "YINI" in hex

// YMETA file structure
class YMETA
{
public:
    YMETA() = default;
    ~YMETA() = default;
    
    // Serialization
    bool serialize(const Parser& parser, const std::string& output_file);
    bool deserialize(const std::string& input_file);
    
    // Export to YINI text
    std::string toYINI() const;
    
    // Getters
    const std::map<std::string, Section>& getSections() const { return sections; }
    const std::map<std::string, std::shared_ptr<Value>>& getDefines() const { return defines; }
    const std::vector<std::string>& getIncludes() const { return includes; }
    
private:
    // Binary serialization helpers
    void writeHeader(std::ofstream& out);
    void writeString(std::ofstream& out, const std::string& str);
    void writeValue(std::ofstream& out, const std::shared_ptr<Value>& value);
    void writeSection(std::ofstream& out, const Section& section);
    
    bool readHeader(std::ifstream& in);
    std::string readString(std::ifstream& in);
    std::shared_ptr<Value> readValue(std::ifstream& in);
    Section readSection(std::ifstream& in);
    
    // Data
    std::map<std::string, Section> sections;
    std::map<std::string, std::shared_ptr<Value>> defines;
    std::vector<std::string> includes;
    
    uint32_t version;
};

} // namespace yini

#endif // YINI_YMETA_H
