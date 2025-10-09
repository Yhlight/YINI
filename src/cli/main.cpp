#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include "parser.h"
#include <nlohmann/json.hpp>
#include "Ymeta/YmetaManager.h"
#include "repl.h"

void print_usage() {
    std::cout << "Usage: yini [options] <file_path>" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --validate         Validate the YINI file." << std::endl;
    std::cout << "  --export-json      Export the YINI file to JSON." << std::endl;
    std::cout << "  --query <query>    Query a specific value from the YINI file." << std::endl;
    std::cout << "  --generate-ymeta   Generate a .ymeta file." << std::endl;
    std::cout << "  --interactive      Enter interactive REPL mode." << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage();
        return 1;
    }

    std::vector<std::string> args(argv + 1, argv + argc);
    std::vector<std::string> commands;
    std::string file_path;
    std::string query;
    bool interactive_mode = false;

    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--validate" || args[i] == "--export-json" || args[i] == "--generate-ymeta") {
            commands.push_back(args[i].substr(2));
        } else if (args[i] == "--interactive") {
            interactive_mode = true;
        } else if (args[i] == "--query") {
            commands.push_back("query");
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

    if (file_path.empty() && !interactive_mode) {
        std::cerr << "Error: No file path provided." << std::endl;
        print_usage();
        return 1;
    }

    try {
        Config config;
        Parser parser;
        YmetaManager ymeta_manager;

        if (!file_path.empty()) {
            auto cached_config = ymeta_manager.read(file_path);
            if (cached_config) {
                config = std::move(*cached_config);
                if (!interactive_mode) std::cout << "Loaded configuration from .ymeta cache." << std::endl;
            } else {
                config = parser.parseFile(file_path);
            }
        }

        if (interactive_mode) {
            run_repl(config, file_path);
        } else {
            if (commands.empty()) {
                std::cout << "File parsed successfully." << std::endl;
            }
            for (const auto& command : commands) {
                if (command == "validate") {
                    try {
                        parser.validate(config);
                        std::cout << "Validation successful." << std::endl;
                    } catch (const std::exception& e) {
                        std::cerr << "Validation failed: " << e.what() << std::endl;
                        return 1;
                    }
                } else if (command == "export-json") {
                    nlohmann::json j = config;
                    std::cout << j.dump(4) << std::endl;
                } else if (command == "generate-ymeta") {
                    ymeta_manager.write(file_path, config);
                    std::cout << ".ymeta file generated successfully." << std::endl;
                } else if (command == "query") {
                    std::string section_name;
                    std::string key_name;
                    size_t dot_pos = query.find('.');
                    if (dot_pos == std::string::npos) {
                        section_name = query;
                    } else {
                        section_name = query.substr(0, dot_pos);
                        key_name = query.substr(dot_pos + 1);
                    }

                    if (config.count(section_name)) {
                        const auto& section = config.at(section_name);
                        if (key_name.empty()) {
                            nlohmann::json j = section;
                            std::cout << j.dump(4) << std::endl;
                        } else {
                            if (section.count(key_name)) {
                                const auto& value = section.at(key_name);
                                nlohmann::json j = value;
                                std::cout << j.dump(4) << std::endl;
                            } else {
                                std::cerr << "Error: Key '" << key_name << "' not found in section '" << section_name << "'." << std::endl;
                                return 1;
                            }
                        }
                    } else {
                        std::cerr << "Error: Section '" << section_name << "' not found." << std::endl;
                        return 1;
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}