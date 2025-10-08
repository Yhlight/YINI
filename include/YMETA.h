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
// YmetaContentFlags:
// - YMETA_CONTENT_FULL: The file contains a full snapshot of a YINI file.
// - YMETA_CONTENT_DYNAMIC_ONLY: The file contains only updated dynamic values.
enum YmetaContentFlags : uint32_t
{
    YMETA_CONTENT_FULL = 1 << 0,
    YMETA_CONTENT_DYNAMIC_ONLY = 1 << 1,
};

class YMETA
{
public:
    YMETA();
    ~YMETA() = default;
    
    // New serialization methods
    void populateFromParser(const Parser& parser);
    bool save(const std::string& output_file, uint32_t flags) const;
    bool load(const std::string& input_file);

    // Dynamic value updates
    void updateDynamicValue(const std::string& key, const std::shared_ptr<Value>& value);

    // Deprecated serialization methods
    [[deprecated("Use populateFromParser and save instead")]]
    bool serialize(const Parser& parser, const std::string& output_file);
    [[deprecated("Use load instead")]]
    bool deserialize(const std::string& input_file);
    
    // Export to YINI text
    std::string toYINI() const;
    
    // Getters
    const std::map<std::string, Section>& getSections() const { return sections; }
    const std::map<std::string, std::shared_ptr<Value>>& getDefines() const { return defines; }
    const std::vector<std::string>& getIncludes() const { return includes; }
    const std::map<std::string, std::vector<std::shared_ptr<Value>>>& getDynamicValues() const { return dynamic_values; }

    // YINI file updating
    bool mergeUpdatesIntoYiniFile(const std::string& yini_input_path, const std::string& yini_output_path) const;
    
private:
    // Binary serialization helpers
    void writeHeader(std::ofstream& out, uint32_t flags) const;
    void writeString(std::ofstream& out, const std::string& str) const;
    void writeValue(std::ofstream& out, const std::shared_ptr<Value>& value) const;
    void writeSection(std::ofstream& out, const Section& section) const;
    
    bool readHeader(std::ifstream& in, uint32_t& flags);
    std::string readString(std::ifstream& in);
    std::shared_ptr<Value> readValue(std::ifstream& in);
    Section readSection(std::ifstream& in);
    
    // Data
    std::map<std::string, Section> sections;
    std::map<std::string, std::shared_ptr<Value>> defines;
    std::vector<std::string> includes;
    std::map<std::string, std::vector<std::shared_ptr<Value>>> dynamic_values;
    
    uint32_t version;
    static constexpr size_t MAX_DYNAMIC_HISTORY = 5;
};

} // namespace yini

#endif // YINI_YMETA_H
