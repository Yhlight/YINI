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
        explicit YiniManager(const std::string& yiniFilePath);

        // Provides read-only access to the document
        const YiniDocument& getDocument() const;
        bool isLoaded() const;

        // Methods to modify values, which will trigger persistence to .ymeta
        void setStringValue(const std::string& section, const std::string& key, const std::string& value);
        void setIntValue(const std::string& section, const std::string& key, int value);
        void setDoubleValue(const std::string& section, const std::string& key, double value);
        void setBoolValue(const std::string& section, const std::string& key, bool value);

        // Writes the modified dynamic values from the .ymeta cache back to the original .yini file.
        bool writeBack();

    private:
        bool load();
        bool save(); // Saves the current document state to the .ymeta file

        std::string yiniFilePath;
        std::string ymetaFilePath;
        YiniDocument doc;
        bool is_loaded;
    };
}

#endif // YINI_MANAGER_HPP