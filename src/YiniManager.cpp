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

    YiniManager::YiniManager(const std::string& yiniFilePath)
        : m_yiniFilePath(yiniFilePath), m_ymetaFilePath(get_ymeta_path(yiniFilePath)), m_isLoaded(false)
    {
        m_isLoaded = load();
    }

    const YiniDocument& YiniManager::getDocument() const
    {
        return m_doc;
    }

    bool YiniManager::isLoaded() const
    {
        return m_isLoaded;
    }

    bool YiniManager::load()
    {
        std::string ymetaContent = read_file_content(m_ymetaFilePath);
        if (!ymetaContent.empty()) {
            if (JsonDeserializer::deserialize(ymetaContent, m_doc)) {
                return true;
            }
        }

        std::string yiniContent = read_file_content(m_yiniFilePath);
        if (yiniContent.empty()) {
            return false;
        }

        try
        {
            m_doc = {};
            std::string basePath = ".";
            size_t last_slash_idx = m_yiniFilePath.rfind('/');
            if (std::string::npos != last_slash_idx)
            {
                basePath = m_yiniFilePath.substr(0, last_slash_idx);
            }

            Parser parser(yiniContent, m_doc, basePath);
            parser.parse();
        }
        catch(...)
        {
            return false;
        }

        return save();
    }

    bool YiniManager::save()
    {
        std::ofstream outFile(m_ymetaFilePath);
        if (!outFile.is_open()) {
            return false;
        }
        std::string jsonContent = JsonSerializer::serialize(m_doc);
        outFile << jsonContent;
        return true;
    }

    static void set_value_helper(YiniDocument& doc, const std::string& section_name, const std::string& key, const YiniValue& value)
    {
        auto* sec = doc.getOrCreateSection(section_name);
        auto it = std::find_if(sec->pairs.begin(), sec->pairs.end(), [&](const auto& p){ return p.key == key; });
        if (it != sec->pairs.end()) {
            it->value = value;
        } else {
            YiniKeyValuePair new_pair;
            new_pair.key = key;
            new_pair.value = value;
            sec->pairs.push_back(std::move(new_pair));
        }
    }

    void YiniManager::setStringValue(const std::string& section, const std::string& key, const std::string& value)
    {
        YiniValue val;
        val.data = value;
        set_value_helper(m_doc, section, key, val);
        save();
    }

    void YiniManager::setIntValue(const std::string& section, const std::string& key, int value)
    {
        YiniValue val;
        val.data = value;
        set_value_helper(m_doc, section, key, val);
        save();
    }

    void YiniManager::setDoubleValue(const std::string& section, const std::string& key, double value)
    {
        YiniValue val;
        val.data = value;
        set_value_helper(m_doc, section, key, val);
        save();
    }

    void YiniManager::setBoolValue(const std::string& section, const std::string& key, bool value)
    {
        YiniValue val;
        val.data = value;
        set_value_helper(m_doc, section, key, val);
        save();
    }
}