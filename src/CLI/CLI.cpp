#include "CLI.h"
#include <iostream>

namespace yini
{

CLI::CLI() : running_(true)
{
    (void)running_; // Suppress unused warning for now
}

void CLI::run()
{
    std::cout << "YINI CLI v1.0.0\n";
    std::cout << "Type 'help' for available commands\n\n";
    
    // TODO: Implement event loop
}

void CLI::processCommand(const std::string& command)
{
    (void)command; // Suppress unused warning for now
    // TODO: Implement command processing
}

} // namespace yini
