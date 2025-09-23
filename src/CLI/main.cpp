#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "yini.h"

// Helper to read a whole file into a string
std::string readFile(const std::string& filepath)
{
    std::ifstream file(filepath);
    if (!file)
    {
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void command_check(const std::string& filepath)
{
    if (filepath.empty())
    {
        std::cout << "Usage: check <filepath>" << std::endl;
        return;
    }

    std::cout << "Checking file: " << filepath << std::endl;

    YINI_HANDLE handle = yini_load_from_string(readFile(filepath).c_str());
    if (!handle) {
        std::cout << "Error: Failed to create YINI handle." << std::endl;
        return;
    }

    int error_count = yini_get_error_count(handle);
    if (error_count > 0) {
        std::cout << "Found " << error_count << " syntax errors:" << std::endl;
        for (int i = 0; i < error_count; ++i) {
            char buffer[256];
            int line, col;
            if (yini_get_error_details(handle, i, buffer, 256, &line, &col)) {
                std::cout << "  - Error (L" << line << ":C" << col << "): " << buffer << std::endl;
            }
        }
    } else {
        std::cout << "Syntax OK." << std::endl;
    }
    yini_free(handle);
}

void command_compile(const std::string& infile, const std::string& outfile)
{
    if (infile.empty() || outfile.empty()) {
        std::cout << "Usage: compile <infile.yini> <outfile.ymeta>" << std::endl;
        return;
    }
    std::cout << "Compiling " << infile << " to " << outfile << std::endl;

    YINI_HANDLE handle = yini_load_from_file(infile.c_str());
    if (!handle) {
        std::cout << "Error: Could not load " << infile << std::endl;
        return;
    }

    if (yini_save_to_file(handle, outfile.c_str())) {
        std::cout << "Compilation successful." << std::endl;
    } else {
        std::cout << "Error during compilation/serialization." << std::endl;
    }
    yini_free(handle);
}

void command_decompile(const std::string& infile, const std::string& outfile)
{
    if (infile.empty() || outfile.empty()) {
        std::cout << "Usage: decompile <infile.ymeta> <outfile.yini>" << std::endl;
        return;
    }
    std::cout << "Decompiling " << infile << " to " << outfile << std::endl;

    // The dump feature was not re-enabled after the refactor.
    // This command is a stub.
    std::cout << "Decompilation is not currently supported." << std::endl;
}

void print_help()
{
    std::cout << "YINI Command-Line Interface" << std::endl;
    std::cout << "Available commands:" << std::endl;
    std::cout << "  check <filepath>        - Check the syntax of a .yini file." << std::endl;
    std::cout << "  compile <in> <out>      - Compile a .yini file to .ymeta." << std::endl;
    std::cout << "  decompile <in> <out>    - (Disabled) Decompile a .ymeta file to .yini." << std::endl;
    std::cout << "  help                    - Show this help message." << std::endl;
    std::cout << "  exit                    - Exit the application." << std::endl;
}

int main()
{
    std::string line;
    print_help();

    while (true)
    {
        std::cout << "> ";
        if (!std::getline(std::cin, line))
        {
            break;
        }

        std::stringstream ss(line);
        std::string command;
        ss >> command;

        if (command == "exit")
        {
            break;
        }
        else if (command == "help")
        {
            print_help();
        }
        else if (command == "check")
        {
            std::string filepath;
            ss >> filepath;
            command_check(filepath);
        }
        else if (command == "compile")
        {
            std::string infile, outfile;
            ss >> infile >> outfile;
            command_compile(infile, outfile);
        }
        else if (command == "decompile")
        {
            std::string infile, outfile;
            ss >> infile >> outfile;
            command_decompile(infile, outfile);
        }
        else if (!command.empty())
        {
            std::cout << "Unknown command: " << command << ". Type 'help' for a list of commands." << std::endl;
        }
    }

    std::cout << "Exiting YINI CLI." << std::endl;
    return 0;
}
