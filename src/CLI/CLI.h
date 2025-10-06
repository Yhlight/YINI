#ifndef YINI_CLI_CLI_H
#define YINI_CLI_CLI_H

#include <string>

namespace yini
{

// CLI class (placeholder)
class CLI
{
public:
    CLI();
    void run();
    
private:
    bool running_;
    void processCommand(const std::string& command);
};

} // namespace yini

#endif // YINI_CLI_CLI_H
