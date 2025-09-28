#include "YINI/YiniManager.hpp"
#include "YINI/Parser.hpp"
#include "YINI/JsonSerializer.hpp"
#include <fstream>
#include <string>

namespace YINI
{
    static std::string read_file_content(const std::string& path) {
        std::ifstream t(path);
        if (!t.is_open()) return "";
        return std::string((std::istreambuf_iterator<char>(t)),
                         std::istreambuf_iterator<char>());
    }

    bool YiniManager::saveYmeta(const std::string& yiniFilePath, const YiniDocument& doc)
    {
        std::string ymetaPath = yiniFilePath;
        size_t dotPos = ymetaPath.rfind(".yini");
        if (dotPos != std::string::npos) {
            ymetaPath.replace(dotPos, 5, ".ymeta");
        } else {
            ymetaPath += ".ymeta";
        }

        std::ofstream outFile(ymetaPath);
        if (!outFile.is_open()) {
            return false;
        }

        std::string jsonContent = JsonSerializer::serialize(doc);
        outFile << jsonContent;
        return true;
    }

    bool YiniManager::loadFromFile(const std::string& filePath, YiniDocument& doc)
    {
        std::string content = read_file_content(filePath);
        if (content.empty()) {
            return false;
        }

        try
        {
            std::string basePath = ".";
            size_t last_slash_idx = filePath.rfind('/');
            if (std::string::npos != last_slash_idx)
            {
                basePath = filePath.substr(0, last_slash_idx);
            }

            Parser parser(content, doc, basePath);
            parser.parse();
        }
        catch(...)
        {
            return false;
        }

        return saveYmeta(filePath, doc);
    }
}