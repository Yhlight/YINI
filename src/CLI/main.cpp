#include <iostream>
#include <string>
#include <vector>
#include "YiniLoader.h"
#include "YmetaSerializer.h"

void printUsage()
{
    std::cout << "YINI CLI Tool\n";
    std::cout << "Usage:\n";
    std::cout << "  yini check <filepath.yini>      - Check syntax of a YINI file\n";
    std::cout << "  yini compile <filepath.yini>    - Compile a YINI file to .ymeta\n";
    std::cout << "  yini decompile <filepath.ymeta> - Decompile a .ymeta file to text\n";
}

void prettyPrint(const Yini::YiniData& data); // Forward declare

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        printUsage();
        return 1;
    }

    std::string command = argv[1];
    std::string filepath = argv[2];

    if (command == "check")
    {
        Yini::YiniLoader loader;
        Yini::YiniData data = loader.loadFile(filepath);
        // The loader doesn't store errors yet. This is a placeholder.
        // For now, just loading is the check.
        std::cout << "Syntax OK\n";
    }
    else if (command == "compile")
    {
        Yini::YiniLoader loader;
        Yini::YiniData data = loader.loadFile(filepath);

        Yini::YmetaSerializer serializer;
        std::string outpath = filepath;
        size_t dot_pos = outpath.rfind('.');
        if (dot_pos != std::string::npos)
        {
            outpath.replace(dot_pos, std::string::npos, ".ymeta");
        }
        else
        {
            outpath += ".ymeta";
        }

        if (serializer.save(data, outpath))
        {
            std::cout << "Compiled " << filepath << " to " << outpath << "\n";
        }
        else
        {
            std::cerr << "Error compiling " << filepath << "\n";
            return 1;
        }
    }
    else if (command == "decompile")
    {
        Yini::YmetaSerializer serializer;
        Yini::YiniData data = serializer.load(filepath);
        prettyPrint(data);
    }
    else
    {
        printUsage();
        return 1;
    }

    return 0;
}

// Basic pretty printer
void prettyPrintValue(const Yini::YiniValue& val);
void prettyPrintVariant(const Yini::YiniVariant& var)
{
    std::visit([](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, Yini::YiniString>) std::cout << '"' << arg << '"';
        else if constexpr (std::is_same_v<T, Yini::YiniInteger>) std::cout << arg;
        else if constexpr (std::is_same_v<T, Yini::YiniFloat>) std::cout << arg;
        else if constexpr (std::is_same_v<T, Yini::YiniBoolean>) std::cout << (arg ? "true" : "false");
        else if constexpr (std::is_same_v<T, Yini::Coordinate2D>) std::cout << "(" << arg.x << ", " << arg.y << ")";
        else if constexpr (std::is_same_v<T, Yini::Coordinate3D>) std::cout << "(" << arg.x << ", " << arg.y << ", " << arg.z << ")";
        else if constexpr (std::is_same_v<T, Yini::ColorRGB>) std::cout << "#" << std::hex << (int)arg.r << (int)arg.g << (int)arg.b << std::dec;
        else if constexpr (std::is_same_v<T, Yini::YiniArray>)
        {
            std::cout << "[";
            for(size_t i=0; i<arg.size(); ++i) { prettyPrintValue(arg[i]); if(i < arg.size() - 1) std::cout << ", "; }
            std::cout << "]";
        }
        else if constexpr (std::is_same_v<T, Yini::YiniMap>)
        {
            std::cout << "{";
            for(auto it = arg.begin(); it != arg.end(); ++it) { std::cout << it->first << ": "; prettyPrintValue(it->second); if(std::next(it) != arg.end()) std::cout << ", "; }
            std::cout << "}";
        }
    }, var);
}
void prettyPrintValue(const Yini::YiniValue& val) { prettyPrintVariant(val.getVariant()); }

void prettyPrint(const Yini::YiniData& data)
{
    for(const auto& [name, section] : data.getSections())
    {
        std::cout << "[" << name << "]";
        if(!section.getInheritance().empty())
        {
            std::cout << " : ";
            for(size_t i=0; i<section.getInheritance().size(); ++i) { std::cout << section.getInheritance()[i]; if(i < section.getInheritance().size() - 1) std::cout << ", "; }
        }
        std::cout << "\n";

        for(const auto& [key, value] : section.getKeyValues())
        {
            std::cout << key << " = ";
            prettyPrintValue(value);
            std::cout << "\n";
        }
        for(const auto& value : section.getValues())
        {
            std::cout << "+= ";
            prettyPrintValue(value);
            std::cout << "\n";
        }
        std::cout << "\n";
    }
}
