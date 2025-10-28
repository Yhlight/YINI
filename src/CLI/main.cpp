#include "CLI.h"
#include <iostream>

int main(int argc, char* argv[])
{
    (void)argc; // Suppress unused warning for now
    (void)argv; // Suppress unused warning for now
    
    yini::CLI cli;
    cli.run();
    return 0;
}
