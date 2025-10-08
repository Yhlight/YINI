#include "YMETA.h"
#include "Parser.h"
#include "Value.h"
#include <cassert>
#include <iostream>
#include <fstream>
#include <string>

using namespace yini;

void test_dynamic_value_update()
{
    std::cout << "Testing dynamic value update and merge..." << std::endl;

    const char* original_yini_path = "test_dynamic.yini";
    const char* ymeta_path = "test_dynamic.ymeta";
    const char* updated_yini_path = "test_dynamic_updated.yini";

    std::string original_content = R"(
[GameState]
level = Dyna(1)
score = 100
player_name = "PlayerOne"
)";

    // 1. Create original YINI file
    std::ofstream(original_yini_path) << original_content;

    // 2. Simulate a game session: load, update dynamic value, and save only dynamic changes
    YMETA ymeta;

    ymeta.updateDynamicValue("GameState.level", std::make_shared<Value>((int64_t)5));

    bool save_success = ymeta.save(ymeta_path, YMETA_CONTENT_DYNAMIC_ONLY);
    assert(save_success && "Failed to save dynamic-only YMETA file.");

    // 3. Simulate game restart: merge updates back into the original YINI file
    YMETA loaded_ymeta;
    bool load_success = loaded_ymeta.load(ymeta_path);
    assert(load_success && "Failed to load dynamic-only YMETA file.");

    bool merge_success = loaded_ymeta.mergeUpdatesIntoYiniFile(original_yini_path, updated_yini_path);
    assert(merge_success && "Failed to merge updates into YINI file.");

    // 4. Verify the updated YINI file
    std::ifstream updated_file(updated_yini_path);
    std::string updated_content((std::istreambuf_iterator<char>(updated_file)),
                                std::istreambuf_iterator<char>());

    Parser updated_parser(updated_content);
    assert(updated_parser.parse() && "Failed to parse updated YINI file.");

    const auto& sections = updated_parser.getSections();
    const auto& game_state = sections.at("GameState");

    auto level_val = game_state.entries.at("level");
    assert(level_val->isInteger() && "Updated value should be an integer.");
    assert(level_val->asInteger() == 5 && "Dynamic value was not updated correctly.");

    auto score_val = game_state.entries.at("score");
    assert(score_val->asInteger() == 100 && "Non-dynamic value was altered.");

    auto name_val = game_state.entries.at("player_name");
    assert(name_val->asString() == "PlayerOne" && "String value was altered.");

    std::cout << "  ✓ Dynamic value correctly updated." << std::endl;
    std::cout << "  ✓ Other values correctly preserved." << std::endl;

    // 5. Cleanup
    remove(original_yini_path);
    remove(ymeta_path);
    remove(updated_yini_path);

    std::cout << "✓ Dynamic value update test passed" << std::endl;
}

int main()
{
    std::cout << "Running Dynamic Update Tests..." << std::endl;
    std::cout << "===============================" << std::endl;

    try
    {
        test_dynamic_value_update();

        std::cout << "\n===============================" << std::endl;
        std::cout << "All dynamic update tests passed! ✓" << std::endl;
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}