#include <iostream>
#include "yini.h"

void run_test()
{
    std::string input = R"yini(
[Section
key = value
)yini";

    std::cout << "--- Parser Error Test ---" << std::endl;
    YINI_HANDLE handle = yini_load_from_string(input.c_str());
    if (!handle) {
        // This shouldn't happen with the new handle design
        std::cerr << "Failed to create handle." << std::endl;
        return;
    }

    int error_count = yini_get_error_count(handle);
    std::cout << "Found " << error_count << " error(s)." << std::endl;

    if (error_count > 0) {
        std::cout << "SUCCESS: Correctly detected parsing errors." << std::endl;
    } else {
        std::cout << "FAILURE: Did not detect parsing errors." << std::endl;
    }

    for (int i = 0; i < error_count; ++i) {
        char buffer[256];
        int line = 0;
        int col = 0;
        if (yini_get_error_details(handle, i, buffer, 256, &line, &col)) {
            std::cout << "  - Error " << i << " (L" << line << ":C" << col << "): " << buffer << std::endl;
        }
    }

    yini_free(handle);
    std::cout << "--- End Parser Error Test ---" << std::endl;
}

int main(int argc, char** argv)
{
    run_test();
    return 0;
}
