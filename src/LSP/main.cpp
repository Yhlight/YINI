#include "LSP/LSPServer.h"
#include <iostream>

int main(int /*argc*/, char* /*argv*/[])
{
    try
    {
        yini::lsp::LSPServer server;
        server.start();
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
}
