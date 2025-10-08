#include "CLI.h"
#include "YMETA.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>

namespace yini
{

CLI::CLI()
    : prompt("yini> ")
    , running(false)
{
    setupBuiltinCommands();
}

CLI::~CLI()
{
}

void CLI::setupBuiltinCommands()
{
    registerCommand(std::make_shared<HelpCommand>(this));
    registerCommand(std::make_shared<ExitCommand>(&running));
    registerCommand(std::make_shared<ParseCommand>());
    registerCommand(std::make_shared<CheckCommand>());
    registerCommand(std::make_shared<CompileCommand>());
    registerCommand(std::make_shared<DecompileCommand>());
}

void CLI::registerCommand(std::shared_ptr<Command> command)
{
    commands[command->getName()] = command;
}

int CLI::run()
{
    printWelcome();
    
    running = true;
    std::string line;
    
    while (running)
    {
        std::cout << prompt;
        std::cout.flush();
        
        if (!std::getline(std::cin, line))
        {
            // EOF or error
            break;
        }
        
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        
        if (line.empty())
        {
            continue;
        }
        
        try
        {
            processLine(line);
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    
    std::cout << "Goodbye!" << std::endl;
    return 0;
}

void CLI::processLine(const std::string& line)
{
    auto args = splitArgs(line);
    
    if (args.empty())
    {
        return;
    }
    
    std::string cmd_name = args[0];
    args.erase(args.begin());
    
    auto it = commands.find(cmd_name);
    if (it != commands.end())
    {
        it->second->execute(args);
    }
    else
    {
        std::cerr << "Unknown command: " << cmd_name << std::endl;
        std::cerr << "Type 'help' for a list of commands." << std::endl;
    }
}

std::vector<std::string> CLI::splitArgs(const std::string& line)
{
    std::vector<std::string> args;
    std::string current;
    bool in_quotes = false;
    
    for (size_t i = 0; i < line.length(); i++)
    {
        char c = line[i];
        
        if (c == '"')
        {
            in_quotes = !in_quotes;
        }
        else if (c == ' ' && !in_quotes)
        {
            if (!current.empty())
            {
                args.push_back(current);
                current.clear();
            }
        }
        else
        {
            current += c;
        }
    }
    
    if (!current.empty())
    {
        args.push_back(current);
    }
    
    return args;
}

void CLI::printWelcome()
{
    std::cout << "╔═══════════════════════════════════════════════╗" << std::endl;
    std::cout << "║      YINI Configuration Language CLI         ║" << std::endl;
    std::cout << "║      Version 1.0.0                           ║" << std::endl;
    std::cout << "╚═══════════════════════════════════════════════╝" << std::endl;
    std::cout << std::endl;
    std::cout << "Type 'help' for a list of commands." << std::endl;
    std::cout << std::endl;
}

void CLI::printHelp()
{
    std::cout << "Available commands:" << std::endl;
    std::cout << std::endl;
    
    for (const auto& [name, cmd] : commands)
    {
        std::cout << "  " << name << std::endl;
        std::cout << "    " << cmd->getHelp() << std::endl;
        std::cout << std::endl;
    }
}

// Built-in command implementations

void HelpCommand::execute(const std::vector<std::string>& args)
{
    (void)args; // Unused
    cli->printHelp();
}

std::string HelpCommand::getHelp() const
{
    return "Show this help message";
}

void ExitCommand::execute(const std::vector<std::string>& args)
{
    (void)args; // Unused
    *running = false;
}

std::string ExitCommand::getHelp() const
{
    return "Exit the CLI";
}

void ParseCommand::execute(const std::vector<std::string>& args)
{
    if (args.empty())
    {
        std::cerr << "Usage: parse <file.yini>" << std::endl;
        return;
    }
    
    std::string filename = args[0];
    
    // Read file
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Error: Could not open file: " << filename << std::endl;
        return;
    }
    
    std::string source((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    file.close();
    
    // Parse
    Parser parser(source);
    bool success = parser.parse();
    
    if (!success)
    {
        std::cerr << "Parse failed: " << parser.getLastError() << std::endl;
        return;
    }
    
    std::cout << "✓ Parse successful!" << std::endl;
    std::cout << std::endl;
    
    // Print statistics
    const auto& sections = parser.getSections();
    const auto& defines = parser.getDefines();
    const auto& includes = parser.getIncludes();
    
    std::cout << "Statistics:" << std::endl;
    std::cout << "  Sections: " << sections.size() << std::endl;
    std::cout << "  Defines: " << defines.size() << std::endl;
    std::cout << "  Includes: " << includes.size() << std::endl;
    std::cout << std::endl;
    
    // Print sections
    if (!sections.empty())
    {
        std::cout << "Sections:" << std::endl;
        for (const auto& [name, section] : sections)
        {
            std::cout << "  [" << name << "]";
            if (!section.inherited_sections.empty())
            {
                std::cout << " : ";
                for (size_t i = 0; i < section.inherited_sections.size(); i++)
                {
                    if (i > 0) std::cout << ", ";
                    std::cout << section.inherited_sections[i];
                }
            }
            std::cout << " (" << section.entries.size() << " entries)" << std::endl;
        }
        std::cout << std::endl;
    }
    
    // Print defines
    if (!defines.empty())
    {
        std::cout << "Defines:" << std::endl;
        for (const auto& [name, value] : defines)
        {
            std::cout << "  @" << name << " = " << value->toString() << std::endl;
        }
        std::cout << std::endl;
    }
}

std::string ParseCommand::getHelp() const
{
    return "Parse a YINI file and display its structure\n"
           "    Usage: parse <file.yini>";
}

void CheckCommand::execute(const std::vector<std::string>& args)
{
    if (args.empty())
    {
        std::cerr << "Usage: check <file.yini>" << std::endl;
        return;
    }
    
    std::string filename = args[0];
    
    // Read file
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Error: Could not open file: " << filename << std::endl;
        return;
    }
    
    std::string source((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    file.close();
    
    // Lex
    std::cout << "Lexing..." << std::endl;
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    
    if (lexer.hasError())
    {
        std::cerr << "✗ Lexer error: " << lexer.getLastError() << std::endl;
        return;
    }
    
    std::cout << "✓ Lexer: " << tokens.size() << " tokens" << std::endl;
    
    // Parse
    std::cout << "Parsing..." << std::endl;
    Parser parser(tokens);
    bool success = parser.parse();
    
    if (!success)
    {
        std::cerr << "✗ Parser error: " << parser.getLastError() << std::endl;
        return;
    }
    
    std::cout << "✓ Parser: successful" << std::endl;
    std::cout << std::endl;
    std::cout << "✓✓ File is valid!" << std::endl;
}

std::string CheckCommand::getHelp() const
{
    return "Check a YINI file for syntax errors\n"
           "    Usage: check <file.yini>";
}

void CompileCommand::execute(const std::vector<std::string>& args)
{
    if (args.empty())
    {
        std::cerr << "Usage: compile <file.yini> [output.ymeta]" << std::endl;
        return;
    }
    
    std::string input_file = args[0];
    std::string output_file = args.size() > 1 ? args[1] : (input_file + ".ymeta");
    
    std::cout << "Compiling: " << input_file << " -> " << output_file << std::endl;
    
    // Read file
    std::ifstream file(input_file);
    if (!file.is_open())
    {
        std::cerr << "Error: Could not open file: " << input_file << std::endl;
        return;
    }
    
    std::string source((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    file.close();
    
    // Parse
    Parser parser(source);
    if (!parser.parse())
    {
        std::cerr << "Parse failed: " << parser.getLastError() << std::endl;
        return;
    }
    
    // Serialize to YMETA
    YMETA ymeta;
    ymeta.populateFromParser(parser);
    if (ymeta.save(output_file, YMETA_CONTENT_FULL))
    {
        std::cout << "✓ Compilation successful!" << std::endl;
        std::cout << "  Output: " << output_file << std::endl;
    }
    else
    {
        std::cerr << "✗ Compilation failed" << std::endl;
    }
}

std::string CompileCommand::getHelp() const
{
    return "Compile a YINI file to YMETA binary format\n"
           "    Usage: compile <file.yini> [output.ymeta]";
}

void DecompileCommand::execute(const std::vector<std::string>& args)
{
    if (args.empty())
    {
        std::cerr << "Usage: decompile <file.ymeta> [output.yini]" << std::endl;
        return;
    }
    
    std::string input_file = args[0];
    std::string output_file = args.size() > 1 ? args[1] : "";
    
    std::cout << "Decompiling: " << input_file;
    if (!output_file.empty())
    {
        std::cout << " -> " << output_file;
    }
    std::cout << std::endl;
    
    // Deserialize YMETA
    YMETA ymeta;
    if (!ymeta.load(input_file))
    {
        std::cerr << "✗ Decompilation failed" << std::endl;
        return;
    }
    
    // Convert to YINI text
    std::string yini_text = ymeta.toYINI();
    
    if (output_file.empty())
    {
        // Print to stdout
        std::cout << std::endl;
        std::cout << yini_text;
    }
    else
    {
        // Write to file
        std::ofstream out(output_file);
        if (!out.is_open())
        {
            std::cerr << "Error: Could not open output file: " << output_file << std::endl;
            return;
        }
        
        out << yini_text;
        out.close();
        
        std::cout << "✓ Decompilation successful!" << std::endl;
        std::cout << "  Output: " << output_file << std::endl;
    }
}

std::string DecompileCommand::getHelp() const
{
    return "Decompile a YMETA file back to YINI text format\n"
           "    Usage: decompile <file.ymeta> [output.yini]";
}

} // namespace yini
