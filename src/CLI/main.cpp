#include <iostream>
#include <fstream>
#include <streambuf>
#include <string>
#include <vector>
#include <sstream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <csignal>
#include "YINI/YiniManager.hpp"
#include "YINI/YiniException.hpp"
#include "YINI/Parser.hpp"

void printHelp() {
    std::cout << "YINI CLI\n";
    std::cout << "Usage: yini-cli [command] [filepath]\n\n";
    std::cout << "Commands:\n";
    std::cout << "  check <filepath>   - Checks the syntax of a .yini file.\n";
    std::cout << "  compile <filepath> - Compiles a .yini file to .ymeta.\n";
    std::cout << "  watch <filepath>   - Watches a file for changes and recompiles automatically.\n";
    std::cout << "  help               - Shows this help message.\n\n";
    std::cout << "If no command is provided, the CLI will start in interactive mode.\n";
}

void handleCheck(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file: " << filePath << std::endl;
        return;
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    try {
        YINI::YiniDocument doc;
        std::string basePath = ".";
        size_t last_slash_idx = filePath.rfind('/');
        if (std::string::npos != last_slash_idx)
        {
            basePath = filePath.substr(0, last_slash_idx);
        }
        YINI::Parser parser(content, doc, basePath);
        parser.parse();
        std::cout << "Syntax OK: " << filePath << std::endl;
    } catch (const YINI::YiniException& e) {
        std::cerr << "Syntax Error in " << filePath << " [" << e.getLine() << ":" << e.getColumn() << "]: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
    }
}

void handleCompile(const std::string& filePath) {
    YINI::YiniManager manager(filePath);
    if (manager.isLoaded()) {
        std::cout << "Successfully compiled " << filePath << " to .ymeta" << std::endl;
    } else {
        std::cerr << "Error: Failed to load or compile " << filePath << ". Check for syntax errors or file access issues." << std::endl;
    }
}

volatile sig_atomic_t g_signal_status;
void signal_handler(int signal) {
    g_signal_status = signal;
}

void handleWatch(const std::string& filePath) {
    if (!std::filesystem::exists(filePath)) {
        std::cerr << "Error: File does not exist: " << filePath << std::endl;
        return;
    }

    signal(SIGINT, signal_handler);
    g_signal_status = 0;

    std::cout << "Watching " << filePath << " for changes... (Press Ctrl+C to stop)" << std::endl;
    auto last_write_time = std::filesystem::last_write_time(filePath);
    handleCompile(filePath); // Initial compile

    while (g_signal_status == 0) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        try {
            if (!std::filesystem::exists(filePath)) {
                std::cerr << "\nFile " << filePath << " no longer exists. Stopping watch." << std::endl;
                break;
            }
            auto current_write_time = std::filesystem::last_write_time(filePath);
            if (current_write_time != last_write_time) {
                last_write_time = current_write_time;
                std::cout << "File changed. Recompiling..." << std::endl;
                handleCompile(filePath);
            }
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Error accessing file: " << e.what() << std::endl;
        }
    }
    std::cout << "\nStopped watching." << std::endl;
}

void runInteractiveMode() {
    std::string line;
    printHelp();

    while (true) {
        std::cout << "> ";
        std::getline(std::cin, line);

        std::stringstream ss(line);
        std::string command;
        ss >> command;

        if (command == "exit") {
            break;
        } else if (command == "help") {
            printHelp();
        } else if (command == "check") {
            std::string filePath;
            ss >> filePath;
            if (filePath.empty()) {
                std::cerr << "Usage: check <filepath>" << std::endl;
            } else {
                handleCheck(filePath);
            }
        } else if (command == "compile") {
            std::string filePath;
            ss >> filePath;
            if (filePath.empty()) {
                std::cerr << "Usage: compile <filepath>" << std::endl;
            } else {
                handleCompile(filePath);
            }
        } else if (command == "watch") {
            std::string filePath;
            ss >> filePath;
            if (filePath.empty()) {
                std::cerr << "Usage: watch <filepath>" << std::endl;
            } else {
                handleWatch(filePath);
            }
        } else if (!command.empty()) {
            std::cerr << "Unknown command: " << command << ". Type 'help' for a list of commands." << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        // Non-interactive mode
        std::string command = argv[1];
        if (command == "help") {
            printHelp();
            return 0;
        }

        if (argc < 3) {
            std::cerr << "Usage: yini-cli <command> <filepath>" << std::endl;
            return 1;
        }

        std::string filePath = argv[2];

        if (command == "check") {
            handleCheck(filePath);
        } else if (command == "compile") {
            handleCompile(filePath);
        } else if (command == "watch") {
            handleWatch(filePath);
        } else {
            std::cerr << "Unknown command: " << command << std::endl;
            printHelp();
            return 1;
        }
    } else {
        // Interactive mode
        runInteractiveMode();
    }

    return 0;
}