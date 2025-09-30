#include "YINI/JsonDeserializer.hpp"
#include "YINI/Parser.hpp"
#include "YINI/YiniException.hpp"
#include "YINI/JsonDeserializer.hpp"
#include "YINI/Parser.hpp"
#include "YINI/YiniException.hpp"
#include "YINI/YiniManager.hpp"
#include "YINI/YiniFormatter.hpp"
#include "YiniValueToString.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <streambuf>
#include <string>
#include <vector>
#include <functional>
#include <map>

// Command handler function signature
using CommandHandler = std::function<bool(const std::vector<std::string>&)>;

// Command dispatch table
static std::map<std::string, CommandHandler> command_handlers;

void printHelp()
{
  std::cout << "YINI CLI - A tool for working with YINI files.\n\n";
  std::cout << "Usage:\n";
  std::cout << "  yini-cli <command> [args...]  (Non-interactive mode)\n";
  std::cout << "  yini-cli                      (Interactive mode)\n\n";
  std::cout << "Available commands:\n";
  std::cout << "  get <key_path> <filepath> - Gets a value (e.g., Section.key).\n";
  std::cout << "  format <filepath> [--in-place] - Formats a .yini file.\n";
  std::cout << "  check <filepath>          - Checks the syntax of a .yini file.\n";
  std::cout << "  compile <filepath>        - Compiles a .yini file to .ymeta.\n";
  std::cout << "  decompile <filepath>      - Decompiles a .ymeta file to a readable format.\n";
  std::cout << "  help                      - Shows this help message.\n";
  std::cout << "  exit                      - Exits the interactive CLI.\n";
}

bool handleCheck(const std::vector<std::string>& args)
{
    if (args.size() != 1) {
        std::cerr << "Usage: yini-cli check <filepath>\n";
        return false;
    }
    const std::string& filePath = args[0];
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file: " << filePath << std::endl;
        return false;
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    try {
        YINI::YiniDocument doc;
        std::string basePath = ".";
        size_t last_slash_idx = filePath.rfind('/');
        if (std::string::npos != last_slash_idx) {
            basePath = filePath.substr(0, last_slash_idx);
        }
        YINI::Parser parser(content, doc, basePath);
        parser.parse();
        std::cout << "Syntax OK: " << filePath << std::endl;
        return true;
    } catch (const YINI::ParsingException& e) {
        std::cerr << "Syntax Error in " << filePath << " [" << e.getLine() << ":" << e.getColumn() << "]: " << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
        return false;
    }
}

bool handleCompile(const std::vector<std::string>& args)
{
    if (args.size() != 1) {
        std::cerr << "Usage: yini-cli compile <filepath>\n";
        return false;
    }
    const std::string& filePath = args[0];
    YINI::YiniManager manager(filePath);
    if (manager.isLoaded()) {
        std::cout << "Successfully compiled " << filePath << " to .ymeta" << std::endl;
        return true;
    } else {
        std::cerr << "Error: Failed to load or compile " << filePath << ". Check for syntax errors or file access issues." << std::endl;
        return false;
    }
}

bool handleDecompile(const std::vector<std::string>& args)
{
    if (args.size() != 1) {
        std::cerr << "Usage: yini-cli decompile <filepath>\n";
        return false;
    }
    const std::string& filePath = args[0];
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file: " << filePath << std::endl;
        return false;
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    YINI::YiniDocument doc;
    if (YINI::JsonDeserializer::deserialize(content, doc)) {
        std::cout << "--- Decompilation of " << filePath << " ---\n\n";
        // ... (rest of the function is the same)
        const auto &defines = doc.getDefines();
        if (!defines.empty()) {
            std::cout << "[#define]\n";
            for (const auto &define_pair : defines) {
                std::cout << "  " << define_pair.first << " = " << YINI::valueToString(define_pair.second) << "\n";
            }
            std::cout << "\n";
        }

        for (const auto &section : doc.getSections()) {
            std::cout << "[" << section.name << "]";
            if (!section.inheritedSections.empty()) {
                std::cout << " : ";
                for (size_t i = 0; i < section.inheritedSections.size(); ++i) {
                    std::cout << section.inheritedSections[i] << (i < section.inheritedSections.size() - 1 ? ", " : "");
                }
            }
            std::cout << "\n";

            for (const auto &pair : section.pairs) {
                std::cout << "  " << pair.key << " = " << YINI::valueToString(pair.value) << "\n";
            }
            for (const auto &val : section.registrationList) {
                std::cout << "  += " << YINI::valueToString(val) << "\n";
            }
            std::cout << "\n";
        }
        std::cout << "--- End of Decompilation ---\n";
        return true;
    } else {
        std::cerr << "Error: Failed to decompile " << filePath << ". It may be corrupted or not a valid .ymeta file." << std::endl;
        return false;
    }
}

bool handleHelp(const std::vector<std::string>&) {
    printHelp();
    return true;
}

bool handleGet(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        std::cerr << "Usage: yini-cli get <Section.key> <filepath>\n";
        return false;
    }

    const std::string& key_path = args[0];
    const std::string& filePath = args[1];

    size_t dot_pos = key_path.find('.');
    if (dot_pos == std::string::npos) {
        std::cerr << "Error: Invalid key path format. Expected 'Section.key'.\n";
        return false;
    }

    std::string section_name = key_path.substr(0, dot_pos);
    std::string key_name = key_path.substr(dot_pos + 1);

    YINI::YiniManager manager(filePath);
    if (!manager.isLoaded()) {
        std::cerr << "Error: Could not load file: " << filePath << "\n";
        return false;
    }

    YINI::YiniDocument doc = manager.getDocument();
    const auto* section = doc.findSection(section_name);
    if (!section) {
        std::cerr << "Error: Section '" << section_name << "' not found.\n";
        return false;
    }

    auto it = std::find_if(section->pairs.begin(), section->pairs.end(),
        [&](const YINI::YiniKeyValuePair& p) { return p.key == key_name; });

    if (it != section->pairs.end()) {
        std::cout << YINI::valueToString(it->value) << std::endl;
        return true;
    } else {
        std::cerr << "Error: Key '" << key_name << "' not found in section '" << section_name << "'.\n";
        return false;
    }
}

bool handleFormat(const std::vector<std::string>& args) {
    if (args.empty() || args.size() > 2) {
        std::cerr << "Usage: yini-cli format <filepath> [--in-place]\n";
        return false;
    }

    const std::string& filePath = args[0];
    bool in_place = (args.size() == 2 && args[1] == "--in-place");

    YINI::YiniManager manager(filePath);
    if (!manager.isLoaded()) {
        std::cerr << "Error: Could not load file: " << filePath << "\n";
        return false;
    }

    YINI::YiniDocument doc = manager.getDocument();
    std::string formatted_content = YINI::YiniFormatter::format(doc);

    if (in_place) {
        std::ofstream outFile(filePath, std::ios::trunc);
        if (!outFile.is_open()) {
            std::cerr << "Error: Could not open file for writing: " << filePath << "\n";
            return false;
        }
        outFile << formatted_content;
        std::cout << "Formatted file in-place: " << filePath << std::endl;
    } else {
        std::cout << formatted_content;
    }

    return true;
}

void initialize_commands() {
    command_handlers["get"] = handleGet;
    command_handlers["format"] = handleFormat;
    command_handlers["check"] = handleCheck;
    command_handlers["compile"] = handleCompile;
    command_handlers["decompile"] = handleDecompile;
    command_handlers["help"] = handleHelp;
    command_handlers["--help"] = handleHelp;
    command_handlers["-h"] = handleHelp;
}

void runInteractiveMode()
{
  std::string line;
  printHelp();

  while (true)
  {
    std::cout << "> ";
    std::getline(std::cin, line);

    std::stringstream ss(line);
    std::string command;
    ss >> command;

    if (command == "exit")
    {
      break;
    }
    else if (command == "help")
    {
      printHelp();
    }
    else if (command == "check")
    {
      std::string filePath;
      ss >> filePath;
      if (filePath.empty())
      {
        std::cerr << "Usage: check <filepath>" << std::endl;
      }
      else
      {
        handleCheck({filePath});
      }
    }
    else if (command == "compile")
    {
      std::string filePath;
      ss >> filePath;
      if (filePath.empty())
      {
        std::cerr << "Usage: compile <filepath>" << std::endl;
      }
      else
      {
        handleCompile({filePath});
      }
    }
    else if (command == "decompile")
    {
      std::string filePath;
      ss >> filePath;
      if (filePath.empty())
      {
        std::cerr << "Usage: decompile <filepath>" << std::endl;
      }
      else
      {
        handleDecompile({filePath});
      }
    }
    else if (!command.empty())
    {
      std::cerr << "Unknown command: " << command
                << ". Type 'help' for a list of commands." << std::endl;
    }
  }
}


int main(int argc, char *argv[])
{
  initialize_commands();

  if (argc > 1)
  {
    std::string command_name = argv[1];

    if (command_handlers.count(command_name)) {
        std::vector<std::string> args;
        for (int i = 2; i < argc; ++i) {
            args.push_back(argv[i]);
        }
        bool success = command_handlers[command_name](args);
        return success ? 0 : 1;
    } else {
        std::cerr << "Unknown command: " << command_name << std::endl;
        printHelp();
        return 1;
    }
  }
  else
  {
    runInteractiveMode();
  }

  return 0;
}