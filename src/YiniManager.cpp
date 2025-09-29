#include "YINI/YiniManager.hpp"
#include "YINI/Parser.hpp"
#include "YINI/JsonSerializer.hpp"
#include "YINI/JsonDeserializer.hpp"
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

    static std::string get_ymeta_path(const std::string& yiniFilePath) {
        std::string ymetaPath = yiniFilePath;
        size_t dotPos = ymetaPath.rfind(".yini");
        if (dotPos != std::string::npos) {
            ymetaPath.replace(dotPos, 5, ".ymeta");
        } else {
            ymetaPath += ".ymeta";
        }
        return ymetaPath;
    }

    bool YiniManager::saveYmeta(const std::string& yiniFilePath, const YiniDocument& doc)
    {
        std::string ymetaPath = get_ymeta_path(yiniFilePath);
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
        // Prioritize loading from .ymeta cache
        std::string ymetaPath = get_ymeta_path(filePath);
        std::string ymetaContent = read_file_content(ymetaPath);
        if (!ymetaContent.empty()) {
            YiniDocument tempDoc;
            if (JsonDeserializer::deserialize(ymetaContent, tempDoc)) {
                doc = std::move(tempDoc);
                return true;
            }
        }

        // Fallback to .yini file
        std::string yiniContent = read_file_content(filePath);
        if (yiniContent.empty()) {
            return false;
        }

        try
        {
            // Clear the document to ensure it's in a clean state before parsing
            doc = {};
            std::string basePath = ".";
            size_t last_slash_idx = filePath.rfind('/');
            if (std::string::npos != last_slash_idx)
            {
                basePath = filePath.substr(0, last_slash_idx);
            }

            Parser parser(yiniContent, doc, basePath);
            parser.parse();
        }
        catch(...)
        {
            return false;
        }

        // Create the cache file for next time
        return saveYmeta(filePath, doc);
    }
}