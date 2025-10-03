#include <gtest/gtest.h>
#include "Interop/YiniCApi.h"
#include <fstream>
#include <vector>
#include <string>

// Helper to create a temporary file for testing
void create_test_file(const std::string& filename, const std::string& content) {
    std::ofstream outfile(filename);
    outfile << content;
    outfile.close();
}

TEST(CApiTest, CreateAndDestroyManager) {
    void* manager = yini_manager_create();
    ASSERT_NE(manager, nullptr);
    yini_manager_destroy(manager);
}

TEST(CApiTest, LoadFile) {
    const std::string filename = "c_api_load_test.yini";
    create_test_file(filename, "[Test]\nkey=val");

    void* manager = yini_manager_create();
    EXPECT_TRUE(yini_manager_load(manager, filename.c_str()));
    yini_manager_destroy(manager);

    // Test with a non-existent file
    void* manager2 = yini_manager_create();
    EXPECT_FALSE(yini_manager_load(manager2, "non_existent_file.yini"));
    yini_manager_destroy(manager2);
}

TEST(CApiTest, GetValues) {
    const std::string filename = "c_api_get_test.yini";
    create_test_file(filename, R"(
        [MySection]
        my_double = 123.45
        my_string = "hello world"
        my_bool_true = true
        my_bool_false = false
    )");

    void* manager = yini_manager_create();
    yini_manager_load(manager, filename.c_str());

    // Test get double
    double d_val;
    EXPECT_TRUE(yini_manager_get_double(manager, "MySection", "my_double", &d_val));
    EXPECT_DOUBLE_EQ(d_val, 123.45);

    // Test get string
    int required_size = yini_manager_get_string(manager, "MySection", "my_string", nullptr, 0);
    ASSERT_GT(required_size, 0);
    std::vector<char> buffer(required_size);
    int written_size = yini_manager_get_string(manager, "MySection", "my_string", buffer.data(), required_size);
    EXPECT_EQ(written_size, required_size - 1);
    EXPECT_STREQ(buffer.data(), "hello world");

    // Test get bool
    bool b_val;
    EXPECT_TRUE(yini_manager_get_bool(manager, "MySection", "my_bool_true", &b_val));
    EXPECT_TRUE(b_val);
    EXPECT_TRUE(yini_manager_get_bool(manager, "MySection", "my_bool_false", &b_val));
    EXPECT_FALSE(b_val);

    // Test non-existent key
    EXPECT_FALSE(yini_manager_get_double(manager, "MySection", "non_existent", &d_val));

    yini_manager_destroy(manager);
}

TEST(CApiTest, SetAndSaveChanges) {
    const std::string filename = "c_api_set_test.yini";
    create_test_file(filename, R"(
        [Settings]
        volume = Dyna(100)
        username = Dyna("player1")
        fullscreen = Dyna(true)
    )");

    void* manager = yini_manager_create();
    yini_manager_load(manager, filename.c_str());

    // Set new values
    yini_manager_set_double(manager, "Settings", "volume", 50.5);
    yini_manager_set_string(manager, "Settings", "username", "player2");
    yini_manager_set_bool(manager, "Settings", "fullscreen", false);

    // Save changes
    yini_manager_save_changes(manager);
    yini_manager_destroy(manager);

    // Load a new manager and verify the changes were saved
    void* verify_manager = yini_manager_create();
    yini_manager_load(verify_manager, filename.c_str());

    double d_val;
    EXPECT_TRUE(yini_manager_get_double(verify_manager, "Settings", "volume", &d_val));
    EXPECT_DOUBLE_EQ(d_val, 50.5);

    char buffer[20];
    yini_manager_get_string(verify_manager, "Settings", "username", buffer, 20);
    EXPECT_STREQ(buffer, "player2");

    bool b_val;
    EXPECT_TRUE(yini_manager_get_bool(verify_manager, "Settings", "fullscreen", &b_val));
    EXPECT_FALSE(b_val);

    yini_manager_destroy(verify_manager);
}