#ifndef YINI_MANAGER_HPP
#define YINI_MANAGER_HPP

#include "YiniData.hpp"
#include <string>
#include <memory>

namespace YINI
{
class YiniManager
{
  public:
    // Constructor loads a YINI document from a given .yini file path.
    // It prioritizes the .ymeta cache if it exists.
    explicit YiniManager(const std::string &yiniFilePath);

    // Provides read-only access to the document
    const YiniDocument &getDocument() const;
    bool isLoaded() const;

    // Methods to modify values, which will trigger persistence to .ymeta
    void setStringValue(const std::string &section, const std::string &key, const std::string &value);
    void setIntValue(const std::string &section, const std::string &key, int value);
    void setDoubleValue(const std::string &section, const std::string &key, double value);
    void setBoolValue(const std::string &section, const std::string &key, bool value);

  private:
    bool load();
    bool save(); // Saves the current document state to the .ymeta file

    std::string m_yiniFilePath;
    std::string m_ymetaFilePath;
    YiniDocument m_doc;
    bool m_isLoaded;
};
} // namespace YINI

#endif // YINI_MANAGER_HPP