#pragma once

#include <string>
#include <vector>

class CLI
{
public:
    void run();

private:
    void printWelcomeMessage();
    void mainLoop();
    void processInput(const std::string& line);

    void cmdCompile(const std::vector<std::string>& args);
    void cmdDecompile(const std::vector<std::string>& args);
    void cmdCheck(const std::vector<std::string>& args);
    void cmdHelp();
};
