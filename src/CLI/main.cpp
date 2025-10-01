#include "Core/YiniManager.h"
#include <iostream>
#include <string>
#include <vector>

void print_usage() {
    std::cerr << "Usage: yini-cli check <filepath>" << std::endl;
}

int main(int argc, char* argv[])
{
    if (argc != 3) {
        print_usage();
        return 1;
    }

    std::string command = argv[1];
    std::string filepath = argv[2];

    if (command != "check") {
        print_usage();
        return 1;
    }

    try {
        YINI::YiniManager manager;
        manager.load(filepath);
        std::cout << "File '" << filepath << "' is valid." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}