#include "repl.h"
#include <sstream>
#include <iostream>
#include "Ymeta/YmetaManager.h"
#include <nlohmann/json.hpp>

std::string process_repl_command(const std::string& line, Config& config, const std::string& filepath) {
    std::stringstream ss(line);
    std::string command;
    ss >> command;
    std::stringstream output;

    if (command == "help") {
        output << "Available commands:\n"
               << "  get <section>.<key>          - Get a value.\n"
               << "  set <section>.<key> <value>  - Set a value (strings must be in quotes).\n"
               << "  save                         - Save changes to the file.\n"
               << "  help                         - Show this help message.\n"
               << "  exit                         - Exit the interactive mode.\n";
    } else if (command == "get") {
        std::string query;
        ss >> query;
        if (query.empty()) {
            output << "Usage: get <section>.<key>";
        } else {
            size_t dot_pos = query.find('.');
            if (dot_pos == std::string::npos) {
                output << "Invalid query format. Use <section>.<key>";
            } else {
                std::string section_name = query.substr(0, dot_pos);
                std::string key_name = query.substr(dot_pos + 1);

                if (config.count(section_name) && config.at(section_name).count(key_name)) {
                    const auto& value = config.at(section_name).at(key_name);
                    nlohmann::json j = value;
                    output << j.dump(4);
                } else {
                    output << "Error: Key '" << key_name << "' not found in section '" << section_name << "'.";
                }
            }
        }
    } else if (command == "set") {
        std::string query;
        std::string value_str;
        ss >> query;
        std::getline(ss, value_str);
        value_str.erase(0, value_str.find_first_not_of(" \t\n\r"));

        if (query.empty() || value_str.empty()) {
            output << "Usage: set <section>.<key> <value>";
        } else {
            size_t dot_pos = query.find('.');
            if (dot_pos == std::string::npos) {
                output << "Invalid query format. Use <section>.<key>";
            } else {
                std::string section_name = query.substr(0, dot_pos);
                std::string key_name = query.substr(dot_pos + 1);
                try {
                    Parser temp_parser;
                    ConfigValue new_value = temp_parser.parseValue(value_str);
                    config[section_name][key_name] = std::move(new_value);
                    output << "Value set.";
                } catch (const std::exception& e) {
                    output << "Error setting value: " << e.what();
                }
            }
        }
    } else if (command == "save") {
        if (filepath.empty()) {
            output << "Error: No file specified to save to.";
        } else {
            YmetaManager ymeta_manager;
            try {
                ymeta_manager.write_yini(filepath, config);
                output << "Configuration saved to " << filepath;
            } catch (const std::exception& e) {
                output << "Error saving file: " << e.what();
            }
        }
    } else if (!command.empty() && command != "exit") {
        output << "Unknown command: " << command;
    }
    return output.str();
}

void run_repl(Config& config, const std::string& filepath) {
    std::cout << "YINI Interactive Mode. Type 'help' for commands, 'exit' to quit." << std::endl;
    std::string line;

    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line) || line == "exit") {
            break;
        }
        std::string result = process_repl_command(line, config, filepath);
        if (!result.empty()) {
            std::cout << result << std::endl;
        }
    }
}