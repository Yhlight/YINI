#include "YiniLoader.h"
#include "../Lexer/Lexer.h"
#include "Parser.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace Yini
{
    // Forward declarations for recursive resolution
    void resolveValue(YiniValue& value, const YiniFile& ast, std::set<std::string>& macroChain);

    void resolveArray(YiniArray& arr, const YiniFile& ast, std::set<std::string>& macroChain)
    {
        for (auto& val : arr)
        {
            resolveValue(val, ast, macroChain);
        }
    }

    void resolveValue(YiniValue& value, const YiniFile& ast, std::set<std::string>& macroChain)
    {
        if (std::holds_alternative<YiniMacroRef>(value.value))
        {
            const auto& macroRef = std::get<YiniMacroRef>(value.value);
            if (macroChain.count(macroRef.name))
            {
                throw std::runtime_error("Circular macro reference detected for: " + macroRef.name);
            }
            if (!ast.definesMap.count(macroRef.name))
            {
                throw std::runtime_error("Undefined macro: " + macroRef.name);
            }
            macroChain.insert(macroRef.name);
            value = ast.definesMap.at(macroRef.name); // Replace the YiniValue
            resolveValue(value, ast, macroChain); // Recursively resolve chained macros
            macroChain.erase(macroRef.name);
        }
        else if (std::holds_alternative<YiniArray>(value.value))
        {
            resolveArray(std::get<YiniArray>(value.value), ast, macroChain);
        }
    }

    void Loader::resolveMacros(YiniFile& ast)
    {
        for (auto& [name, section] : ast.sectionsMap)
        {
            for (auto& [key, value] : section.keyValues)
            {
                std::set<std::string> macroChain;
                resolveValue(value, ast, macroChain);
            }
            for (auto& value : section.autoIndexedValues)
            {
                std::set<std::string> macroChain;
                resolveValue(value, ast, macroChain);
            }
        }
    }

    YiniFile Loader::load(const std::string& rootFilepath)
    {
        YiniFile finalAst = parseFile(rootFilepath);

        std::set<std::string> processedFiles;
        processedFiles.insert(rootFilepath);
        processIncludes(finalAst, rootFilepath, processedFiles);

        resolveMacros(finalAst);
        applyInheritance(finalAst);

        return finalAst;
    }

    YiniFile Loader::parseFile(const std::string& filepath)
    {
        std::ifstream file(filepath);
        if (!file)
        {
            throw std::runtime_error("Failed to open file: " + filepath);
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();

        Lexer lexer(content);
        auto tokens = lexer.tokenize();

        Parser parser(std::move(tokens));
        return parser.parse();
    }

    void Loader::merge(YiniFile& baseAst, const YiniFile& includedAst)
    {
        baseAst.definesMap.insert(includedAst.definesMap.begin(), includedAst.definesMap.end());
        for (const auto& [name, section] : includedAst.sectionsMap)
        {
            if (baseAst.sectionsMap.count(name))
            {
                auto& baseSection = baseAst.sectionsMap.at(name);
                baseSection.keyValues.insert(section.keyValues.begin(), section.keyValues.end());
                baseSection.autoIndexedValues.insert(
                    baseSection.autoIndexedValues.end(),
                    section.autoIndexedValues.begin(),
                    section.autoIndexedValues.end()
                );
            }
            else
            {
                baseAst.sectionsMap[name] = section;
            }
        }
    }

    void Loader::processIncludes(YiniFile& ast, const std::string& basePath, std::set<std::string>& processedFiles)
    {
        if (ast.includePaths.empty())
        {
            return;
        }
        std::vector<std::string> includes = ast.includePaths;
        ast.includePaths.clear();

        for (const auto& includePath : includes)
        {
            if (processedFiles.count(includePath))
            {
                continue;
            }
            processedFiles.insert(includePath);
            YiniFile includedAst = parseFile(includePath);
            processIncludes(includedAst, includePath, processedFiles);
            merge(ast, includedAst);
        }
    }

    void Loader::applyInheritance(YiniFile& ast)
    {
        for (auto& [name, section] : ast.sectionsMap)
        {
            std::set<std::string> inheritanceChain;
            applySectionInheritance(section, ast, inheritanceChain);
        }
    }

    void Loader::applySectionInheritance(YiniSection& section, YiniFile& ast, std::set<std::string>& inheritanceChain)
    {
        if (inheritanceChain.count(section.name))
        {
            throw std::runtime_error("Circular inheritance detected for section: " + section.name);
        }
        inheritanceChain.insert(section.name);

        for (const auto& parentName : section.inherits)
        {
            if (!ast.sectionsMap.count(parentName))
            {
                throw std::runtime_error("Parent section '" + parentName + "' not found for child '" + section.name + "'.");
            }

            YiniSection& parentSection = ast.sectionsMap.at(parentName);
            applySectionInheritance(parentSection, ast, inheritanceChain);

            for (const auto& [key, value] : parentSection.keyValues)
            {
                section.keyValues.try_emplace(key, value);
            }
            section.autoIndexedValues.insert(
                section.autoIndexedValues.begin(),
                parentSection.autoIndexedValues.begin(),
                parentSection.autoIndexedValues.end()
            );
        }

        inheritanceChain.erase(section.name);
    }
}
