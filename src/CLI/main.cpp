#include "CLI.h"
#include <iostream>

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    
    try
    {
        yini::CLI cli;
        return cli.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
}
