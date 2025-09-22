#include "YiniLoader.h"
#include "Lexer.h"
#include "Parser.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <set>
#include <filesystem>


namespace Yini
{
    void YiniLoader::mergeData(YiniData& base, const YiniData& child)
    {
        for(const auto& [sectionName, childSection] : child.getSections())
        {
                YiniSection* baseSection = base.getSection(sectionName);
                if(baseSection)
                {
                    // Merge key-values, child overrides base
                    for(const auto& [key, value] : childSection.getKeyValues())
                    {
                        baseSection->addKeyValuePair(key, value);
                    }
                    // Append += values
                    for(const auto& value : childSection.getValues())
                    {
                        baseSection->addValue(value);
                    }
                }
                else
                {
                    base.addSection(childSection);
                }
            }

            for(const auto& [key, value] : child.getMacros())
            {
                base.addMacro(key, value);
            }
    }


    YiniLoader::YiniLoader() {}

    YiniData YiniLoader::loadFile(const std::string& filepath)
    {
        std::set<std::string> loadedFiles;
        return loadFileRecursive(filepath, loadedFiles);
    }

    YiniData YiniLoader::loadFileRecursive(const std::string& filepath, std::set<std::string>& loadedFiles)
    {
        if(loadedFiles.count(filepath))
        {
            // circular dependency
            return {};
        }
        loadedFiles.insert(filepath);

        YiniData data = loadAndParse(filepath);

        std::string basePath = std::filesystem::path(filepath).parent_path().string();
        resolveIncludes(data, basePath, loadedFiles);
        resolveMacros(data);
        resolveInheritance(data);

        return data;
    }


    YiniData YiniLoader::loadAndParse(const std::string& filepath)
    {
        std::ifstream file(filepath);
        if(!file.is_open())
        {
            std::cerr << "Error: could not open file " << filepath << std::endl;
            return {};
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();

        Lexer lexer(content);
        Parser parser(lexer);
        return parser.parseYini();
    }

    void YiniLoader::resolveIncludes(YiniData& data, const std::string& basePath, std::set<std::string>& loadedFiles)
    {
        for(const auto& includePath : data.getIncludes())
        {
            std::string fullPath = basePath + "/" + includePath;
            YiniData includedData = loadFileRecursive(fullPath, loadedFiles);
            mergeData(data, includedData);
        }
    }

    void YiniLoader::resolveInheritance(YiniData& data)
    {
        bool changed = true;
        while(changed)
        {
            changed = false;
            for(auto& [sectionName, section] : data.getSections())
            {
                if(section.getInheritance().empty()) continue;

                for(const auto& parentName : section.getInheritance())
                {
                    const YiniSection* parentSection = data.getSection(parentName);
                    if(parentSection)
                    {
                        for(const auto& [key, value] : parentSection->getKeyValues())
                        {
                            // if key does not exist in child, add it
                            if(section.getKeyValues().find(key) == section.getKeyValues().end())
                            {
                                section.addKeyValuePair(key, value);
                                changed = true;
                            }
                        }
                    }
                }
            }
        }
    }

    void YiniLoader::resolveMacros(YiniData& data)
    {
        // TODO
    }
}
