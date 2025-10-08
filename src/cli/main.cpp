#include <iostream>
#include <string>
#include <vector>
#include "parser.h"

void print_usage() {
    std::cout << "Usage: yini [options] <file_path>" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --validate         Validate the YINI file." << std::endl;
    std::cout << "  --export-json      Export the YINI file to JSON." << std::endl;
    std::cout << "  --query <query>    Query a specific value from the YINI file." << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage();
        return 1;
    }

    std::vector<std::string> args(argv + 1, argv + argc);
    std::string command;
    std::string file_path;
    std::string query;

    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--validate") {
            command = "validate";
        } else if (args[i] == "--export-json") {
            command = "export-json";
        } else if (args[i] == "--query") {
            command = "query";
            if (i + 1 < args.size()) {
                query = args[++i];
            } else {
                std::cerr << "Error: --query requires a query string." << std::endl;
                return 1;
            }
        } else {
            if (file_path.empty()) {
                file_path = args[i];
            } else {
                std::cerr << "Error: Unknown argument or multiple files specified." << std::endl;
                print_usage();
                return 1;
            }
        }
    }

    if (file_path.empty()) {
        std::cerr << "Error: No file path provided." << std::endl;
        print_usage();
        return 1;
    }

    try {
        Parser parser;
        Config config = parser.parseFile(file_path);

        if (command == "validate") {
            try {
                parser.validate(config);
                std::cout << "Validation successful." << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Validation failed: " << e.what() << std::endl;
                return 1;
            }
        } else if (command == "export-json") {
            // Placeholder for future implementation
            std::cout << "JSON export functionality is not yet implemented." << std::endl;
        } else if (command == "query") {
            // Placeholder for future implementation
            std::cout << "Query functionality is not yet implemented." << std::endl;
        } else {
            // Default action: just parse
            std::cout << "File parsed successfully." << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}