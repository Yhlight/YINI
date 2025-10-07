#ifndef YINI_CLI_H
#define YINI_CLI_H

#include "Parser.h"
#include "Lexer.h"
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>

namespace yini
{

// CLI Command interface
class Command
{
public:
    virtual ~Command() = default;
    virtual void execute(const std::vector<std::string>& args) = 0;
    virtual std::string getHelp() const = 0;
    virtual std::string getName() const = 0;
};

// CLI Application
class CLI
{
public:
    CLI();
    ~CLI();
    
    // Main event loop
    int run();
    
    // Command registration
    void registerCommand(std::shared_ptr<Command> command);
    
    // Input/Output
    void setPrompt(const std::string& prompt) { this->prompt = prompt; }
    std::string getPrompt() const { return prompt; }
    
    // Helpers
    void printWelcome();
    void printHelp();
    
private:
    // Command parsing and execution
    void processLine(const std::string& line);
    std::vector<std::string> splitArgs(const std::string& line);
    
    // Built-in commands
    void setupBuiltinCommands();
    
    // State
    std::map<std::string, std::shared_ptr<Command>> commands;
    std::string prompt;
    bool running;
};

// Built-in commands

class HelpCommand : public Command
{
public:
    explicit HelpCommand(CLI* cli) : cli(cli) {}
    
    void execute(const std::vector<std::string>& args) override;
    std::string getHelp() const override;
    std::string getName() const override { return "help"; }
    
private:
    CLI* cli;
};

class ExitCommand : public Command
{
public:
    explicit ExitCommand(bool* running) : running(running) {}
    
    void execute(const std::vector<std::string>& args) override;
    std::string getHelp() const override;
    std::string getName() const override { return "exit"; }
    
private:
    bool* running;
};

class ParseCommand : public Command
{
public:
    ParseCommand() = default;
    
    void execute(const std::vector<std::string>& args) override;
    std::string getHelp() const override;
    std::string getName() const override { return "parse"; }
};

class CheckCommand : public Command
{
public:
    CheckCommand() = default;
    
    void execute(const std::vector<std::string>& args) override;
    std::string getHelp() const override;
    std::string getName() const override { return "check"; }
};

class CompileCommand : public Command
{
public:
    CompileCommand() = default;
    
    void execute(const std::vector<std::string>& args) override;
    std::string getHelp() const override;
    std::string getName() const override { return "compile"; }
};

class DecompileCommand : public Command
{
public:
    DecompileCommand() = default;
    
    void execute(const std::vector<std::string>& args) override;
    std::string getHelp() const override;
    std::string getName() const override { return "decompile"; }
};

} // namespace yini

#endif // YINI_CLI_H
