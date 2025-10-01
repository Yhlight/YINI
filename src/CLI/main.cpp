#include "YINI/JsonDeserializer.hpp"
#include "YINI/Parser.hpp"
#include "YINI/YiniException.hpp"
#include "YINI/YiniManager.hpp"
#include "YiniValueToString.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <streambuf>
#include <string>
#include <vector>

void printHelp()
{
  std::cout << "YINI CLI - A tool for working with YINI files.\n\n";
  std::cout << "Usage:\n";
  std::cout << "  yini-cli <command> <filepath>  (Non-interactive mode)\n";
  std::cout << "  yini-cli                       (Interactive mode)\n\n";
  std::cout << "Available commands:\n";
  std::cout << "  check <filepath>     - Checks the syntax of a .yini file.\n";
  std::cout
      << "  compile <filepath>   - Compiles a .yini file to .ymeta.\n";
  std::cout << "  decompile <filepath> - Decompiles a .ymeta file to a readable "
               "format.\n";
  std::cout << "  help                 - Shows this help message.\n";
  std::cout << "  exit                 - Exits the interactive CLI.\n";
}

bool handleCheck(const std::string &filePath)
{
  std::ifstream file(filePath);
  if (!file.is_open())
  {
    std::cerr << "Error: Could not open file: " << filePath << std::endl;
    return false;
  }
  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());

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
    std::cout << "Syntax OK: " << filePath << std::endl;
    return true;
  }
  catch (const YINI::YiniException &e)
  {
    std::cerr << "Syntax Error in " << filePath << " [" << e.getLine() << ":"
              << e.getColumn() << "]: " << e.what() << std::endl;
    return false;
  }
  catch (const std::exception &e)
  {
    std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
    return false;
  }
}

bool handleCompile(const std::string &filePath)
{
  YINI::YiniManager manager(filePath);
  if (manager.is_loaded())
  {
    std::cout << "Successfully compiled " << filePath << " to .ymeta"
              << std::endl;
    return true;
  }
  else
  {
    std::cerr << "Error: Failed to load or compile " << filePath
              << ". Check for syntax errors or file access issues."
              << std::endl;
    return false;
  }
}

bool handleDecompile(const std::string &filePath)
{
  std::ifstream file(filePath);
  if (!file.is_open())
  {
    std::cerr << "Error: Could not open file: " << filePath << std::endl;
    return false;
  }
  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());

  YINI::YiniDocument doc;
  if (YINI::JsonDeserializer::deserialize(content, doc))
  {
    std::cout << "--- Decompilation of " << filePath << " ---\n\n";

    const auto &defines = doc.getDefines();
    if (!defines.empty())
    {
      std::cout << "[#define]\n";
      for (const auto &define_pair : defines)
      {
        std::cout << "  " << define_pair.first << " = "
                  << YINI::valueToString(define_pair.second) << "\n";
      }
      std::cout << "\n";
    }

    for (const auto &section : doc.getSections())
    {
      std::cout << "[" << section.name << "]";
      if (!section.inheritedSections.empty())
      {
        std::cout << " : ";
        for (size_t i = 0; i < section.inheritedSections.size(); ++i)
        {
          std::cout << section.inheritedSections[i]
                    << (i < section.inheritedSections.size() - 1 ? ", " : "");
        }
      }
      std::cout << "\n";

      for (const auto &pair : section.pairs)
      {
        std::cout << "  " << pair.key << " = " << YINI::valueToString(pair.value)
                  << "\n";
      }
      for (const auto &val : section.registrationList)
      {
        std::cout << "  += " << YINI::valueToString(val) << "\n";
      }
      std::cout << "\n";
    }
    std::cout << "--- End of Decompilation ---\n";
    return true;
  }
  else
  {
    std::cerr << "Error: Failed to decompile " << filePath
              << ". It may be corrupted or not a valid .ymeta file."
              << std::endl;
    return false;
  }
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
        handleCheck(filePath);
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
        handleCompile(filePath);
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
        handleDecompile(filePath);
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
  if (argc > 1)
  {
    std::string command = argv[1];
    if (command == "help" || command == "--help" || command == "-h")
    {
      printHelp();
      return 0;
    }

    if (argc < 3)
    {
      std::cerr << "Error: Missing file path for command '" << command
                << "'.\n";
      std::cerr << "Usage: yini-cli <command> <filepath>\n";
      return 1;
    }
    std::string filePath = argv[2];

    bool success = false;
    if (command == "check")
    {
      success = handleCheck(filePath);
    }
    else if (command == "compile")
    {
      success = handleCompile(filePath);
    }
    else if (command == "decompile")
    {
      success = handleDecompile(filePath);
    }
    else
    {
      std::cerr << "Unknown command: " << command << std::endl;
      printHelp();
      return 1;
    }
    return success ? 0 : 1;
  }
  else
  {
    runInteractiveMode();
  }

  return 0;
}