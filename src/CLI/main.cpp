#include "../Loader.h"
#include "../Json.h"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: yini_cli <input_file.yini>" << std::endl;
        return 1;
    }

    std::string filepath = argv[1];

    YINI::Loader loader;
    try {
        // Load the document, use cache if available and valid
        YINI::Document doc = loader.load(filepath, true);

        // Print the document as JSON to stdout
        std::cout << YINI::Json::to_json(doc) << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
