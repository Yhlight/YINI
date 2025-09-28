#ifndef YINI_MANAGER_HPP
#define YINI_MANAGER_HPP

#include "YiniData.hpp"
#include <string>

namespace YINI
{
    class YiniManager
    {
    public:
        // Loads a YINI document from a given .yini file path.
        // If successful, it also creates/updates the corresponding .ymeta file.
        static bool loadFromFile(const std::string& filePath, YiniDocument& doc);

    private:
        // Helper to save document to a .ymeta file.
        static bool saveYmeta(const std::string& yiniFilePath, const YiniDocument& doc);
    };
}

#endif // YINI_MANAGER_HPP