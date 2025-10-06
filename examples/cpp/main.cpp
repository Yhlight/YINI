#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

// Include the main YINI header.
// This works because the find_package(Yini) command in CMakeLists.txt
// adds the necessary include directory to the target.
#include <Yini.h>

void printSection(const std::string& title) {
    std::cout << "\n--- " << title << " ---\n";
}

int main() {
    // It's good practice to wrap resource-managing objects in a scope.
    try {
        // 1. Create a YiniManager instance.
        Yini::Manager yini;

        // 2. Load the configuration file.
        const std::string configFile = "config.yini";
        if (!yini.load(configFile)) {
            std::cerr << "Error: Could not load file '" << configFile << "'." << std::endl;
            return 1;
        }
        std::cout << "Successfully loaded '" << configFile << "'." << std::endl;

        // 3. Validate the configuration against the embedded schema.
        printSection("Schema Validation");
        if (!yini.validate()) {
            std::cerr << "Schema validation failed!" << std::endl;
            // Print out the validation errors.
            for (const auto& error : yini.getValidationErrors()) {
                std::cerr << "- " << error << std::endl;
            }
            return 1;
        }
        std::cout << "Schema validation successful." << std::endl;

        // 4. Read values from the [Player] section.
        printSection("Reading Player Stats");
        std::string playerName = yini.getString("Player", "name").value_or("Default");
        int playerLevel = yini.getInt("Player", "level").value_or(0);

        std::cout << "Player Name: " << playerName << std::endl;
        std::cout << "Player Level: " << playerLevel << std::endl;

        // Handle optional values gracefully. The `exp` key is optional in the schema.
        if (auto exp = yini.getFloat("Player", "exp")) {
            std::cout << "Player Exp: " << *exp << std::endl;
        } else {
            std::cout << "Player Exp: (not specified)" << std::endl;
        }

        // 5. Read values from the [Graphics] section.
        printSection("Reading Graphics Settings");
        bool isFullscreen = yini.getBool("Graphics", "fullscreen").value_or(false);
        int resX = yini.getInt("Graphics", "resolution_x").value_or(800);
        int resY = yini.getInt("Graphics", "resolution_y").value_or(600);

        std::cout << "Fullscreen: " << (isFullscreen ? "Enabled" : "Disabled") << std::endl;
        std::cout << "Resolution: " << resX << "x" << resY << std::endl;

        // 6. Read a list-like value using the `+=` registration syntax.
        printSection("Reading Achievements");
        if (auto achievements = yini.getList("UnlockedAchievements", "UnlockedAchievements")) {
            std::cout << "Unlocked Achievements:" << std::endl;
            for (const auto& achievementValue : *achievements) {
                // YiniValue holds a std::variant, we can use std::get to access the string.
                if (auto* achievementStr = std::get_if<std::string>(&achievementValue)) {
                    std::cout << "- " << *achievementStr << std::endl;
                }
            }
        }

    } catch (const std::exception& e) {
        // Catch potential exceptions from the YINI library (e.g., during parsing).
        std::cerr << "\nAn unexpected error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}