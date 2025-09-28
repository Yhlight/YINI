#include <iostream>
#include <fstream>
#include <streambuf>
#include "YINI/Parser.hpp"
#include "YINI/YiniException.hpp"

static std::string readFile(const std::string& path) {
    std::ifstream t(path);
    if (!t.is_open()) {
        return "";
    }
    return std::string((std::istreambuf_iterator<char>(t)),
                     std::istreambuf_iterator<char>());
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: yini-cli <file_path>" << std::endl;
        return 1;
    }

    std::string filePath = argv[1];
    std::string content = readFile(filePath);

    if (content.empty())
    {
        std::cerr << "Error: Could not read file or file is empty: " << filePath << std::endl;
        return 1;
    }

    try
    {
        YINI::YiniDocument doc;
        std::string basePath = ".";
        size_t last_slash_idx = filePath.rfind('/');
        if (std::string::npos != last_slash_idx)
        {
            basePath = filePath.substr(0, last_slash_idx);
        }

        YINI::Parser parser(content, doc, basePath);
        parser.parse();

        std::cout << "Syntax check passed for " << filePath << std::endl;
    }
    catch (const YINI::YiniException& e)
    {
        std::cerr << "Syntax Error in " << filePath << " [" << e.getLine() << ":" << e.getColumn() << "]: " << e.what() << std::endl;
        return 1;
    }
    catch (const std::exception& e)
    {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}