#include "YMETA.h"
#include "Parser.h"
#include "Value.h"
#include <cassert>
#include <iostream>
#include <fstream>
#include <string>

using namespace yini;

void test_dynamic_value_history()
{
    std::cout << "Testing dynamic value history and merge..." << std::endl;

    const char* original_yini_path = "test_history.yini";
    const char* ymeta_path = "test_history.ymeta";
    const char* updated_yini_path = "test_history_updated.yini";

    std::string original_content = R"(
[GameState]
level = Dyna(1)
)";

    // 1. Create original YINI file
    std::ofstream(original_yini_path) << original_content;

    // 2. Simulate a game session with multiple updates to a dynamic value
    YMETA ymeta;

    // Update the value 7 times to test the history limit of 5
    ymeta.updateDynamicValue("GameState.level", std::make_shared<Value>((int64_t)2));
    ymeta.updateDynamicValue("GameState.level", std::make_shared<Value>((int64_t)3));
    ymeta.updateDynamicValue("GameState.level", std::make_shared<Value>((int64_t)4));
    ymeta.updateDynamicValue("GameState.level", std::make_shared<Value>((int64_t)5));
    ymeta.updateDynamicValue("GameState.level", std::make_shared<Value>((int64_t)6));
    ymeta.updateDynamicValue("GameState.level", std::make_shared<Value>((int64_t)7));
    ymeta.updateDynamicValue("GameState.level", std::make_shared<Value>((int64_t)8)); // This should be the latest value

    // Save only the dynamic values to the ymeta file
    bool save_success = ymeta.save(ymeta_path, YMETA_CONTENT_DYNAMIC_ONLY);
    assert(save_success && "Failed to save dynamic-only YMETA file.");

    // 3. Simulate game restart: load the ymeta and verify history
    YMETA loaded_ymeta;
    bool load_success = loaded_ymeta.load(ymeta_path);
    assert(load_success && "Failed to load dynamic-only YMETA file.");

    const auto& dynamic_values = loaded_ymeta.getDynamicValues();
    const auto& history = dynamic_values.at("GameState.level");

    assert(history.size() == 5 && "History should be capped at 5 entries.");
    assert(history[0]->asInteger() == 8 && "The latest value should be 8.");
    assert(history[4]->asInteger() == 4 && "The oldest value in history should be 4.");
    std::cout << "  ✓ History correctly saved and loaded." << std::endl;

    // 4. Merge updates back into the original YINI file
    bool merge_success = loaded_ymeta.mergeUpdatesIntoYiniFile(original_yini_path, updated_yini_path);
    assert(merge_success && "Failed to merge updates into YINI file.");

    // 5. Verify the updated YINI file
    std::ifstream updated_file(updated_yini_path);
    std::string updated_content((std::istreambuf_iterator<char>(updated_file)),
                                std::istreambuf_iterator<char>());

    Parser updated_parser(updated_content);
    assert(updated_parser.parse() && "Failed to parse updated YINI file.");

    const auto& sections = updated_parser.getSections();
    const auto& game_state = sections.at("GameState");

    auto level_val = game_state.entries.at("level");
    assert(level_val->asInteger() == 8 && "The latest value from history was not merged correctly.");
    std::cout << "  ✓ Latest value correctly merged." << std::endl;

    // 6. Cleanup
    remove(original_yini_path);
    remove(ymeta_path);
    remove(updated_yini_path);

    std::cout << "✓ Dynamic value history test passed" << std::endl;
}

int main()
{
    std::cout << "Running Dynamic History Tests..." << std::endl;
    std::cout << "===============================" << std::endl;

    try
    {
        test_dynamic_value_history();

        std::cout << "\n===============================" << std::endl;
        std::cout << "All dynamic history tests passed! ✓" << std::endl;
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}