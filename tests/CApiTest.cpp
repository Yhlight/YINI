#include <gtest/gtest.h>
#include "Interop/YiniCApi.h"
#include <fstream>
#include <vector>
#include <string>
#include <thread>

// Helper to create a temporary file for testing
void create_test_file(const std::string& filename, const std::string& content) {
    std::ofstream outfile(filename);
    outfile << content;
    outfile.close();
}

TEST(CApiTest, CreateAndDestroyManager) {
    Yini_ManagerHandle manager = yini_manager_create();
    ASSERT_NE(manager, nullptr);
    yini_manager_destroy(manager);
}

TEST(CApiTest, LoadFile) {
    const std::string filename = "c_api_load_test.yini";
    create_test_file(filename, "[Test]\nkey=val");

    Yini_ManagerHandle manager = yini_manager_create();
    EXPECT_TRUE(yini_manager_load(manager, filename.c_str()));
    yini_manager_destroy(manager);

    // Test with a non-existent file
    Yini_ManagerHandle manager2 = yini_manager_create();
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

    Yini_ManagerHandle manager = yini_manager_create();
    yini_manager_load(manager, filename.c_str());

    // Test get double
    double d_val;
    Yini_ValueHandle d_handle = yini_manager_get_value(manager, "MySection", "my_double");
    ASSERT_NE(d_handle, nullptr);
    EXPECT_EQ(yini_value_get_type(d_handle), YiniValueType_Double);
    EXPECT_TRUE(yini_value_get_double(d_handle, &d_val));
    EXPECT_DOUBLE_EQ(d_val, 123.45);
    yini_value_destroy(d_handle);

    // Test get string
    Yini_ValueHandle s_handle = yini_manager_get_value(manager, "MySection", "my_string");
    ASSERT_NE(s_handle, nullptr);
    EXPECT_EQ(yini_value_get_type(s_handle), YiniValueType_String);
    int required_size = yini_value_get_string(s_handle, nullptr, 0);
    ASSERT_GT(required_size, 0);
    std::vector<char> buffer(required_size);
    int written_size = yini_value_get_string(s_handle, buffer.data(), required_size);
    EXPECT_EQ(written_size, required_size - 1);
    EXPECT_STREQ(buffer.data(), "hello world");
    yini_value_destroy(s_handle);

    // Test get bool
    bool b_val;
    Yini_ValueHandle bt_handle = yini_manager_get_value(manager, "MySection", "my_bool_true");
    ASSERT_NE(bt_handle, nullptr);
    EXPECT_TRUE(yini_value_get_bool(bt_handle, &b_val));
    EXPECT_TRUE(b_val);
    yini_value_destroy(bt_handle);

    Yini_ValueHandle bf_handle = yini_manager_get_value(manager, "MySection", "my_bool_false");
    ASSERT_NE(bf_handle, nullptr);
    EXPECT_TRUE(yini_value_get_bool(bf_handle, &b_val));
    EXPECT_FALSE(b_val);
    yini_value_destroy(bf_handle);

    // Test non-existent key
    Yini_ValueHandle n_handle = yini_manager_get_value(manager, "MySection", "non_existent");
    EXPECT_EQ(n_handle, nullptr);

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

    Yini_ManagerHandle manager = yini_manager_create();
    yini_manager_load(manager, filename.c_str());

    // Set new values
    Yini_ValueHandle double_val = yini_value_create_double(50.5);
    Yini_ValueHandle string_val = yini_value_create_string("player2");
    Yini_ValueHandle bool_val = yini_value_create_bool(false);

    yini_manager_set_value(manager, "Settings", "volume", double_val);
    yini_manager_set_value(manager, "Settings", "username", string_val);
    yini_manager_set_value(manager, "Settings", "fullscreen", bool_val);

    // Destroy the handles now that they've been passed to the manager
    yini_value_destroy(double_val);
    yini_value_destroy(string_val);
    yini_value_destroy(bool_val);

    // Save changes
    yini_manager_save_changes(manager);
    yini_manager_destroy(manager);

    // Load a new manager and verify the changes were saved
    Yini_ManagerHandle verify_manager = yini_manager_create();
    yini_manager_load(verify_manager, filename.c_str());

    double d_val;
    Yini_ValueHandle d_handle_verify = yini_manager_get_value(verify_manager, "Settings", "volume");
    EXPECT_TRUE(yini_value_get_double(d_handle_verify, &d_val));
    EXPECT_DOUBLE_EQ(d_val, 50.5);
    yini_value_destroy(d_handle_verify);

    char buffer[20];
    Yini_ValueHandle s_handle_verify = yini_manager_get_value(verify_manager, "Settings", "username");
    yini_value_get_string(s_handle_verify, buffer, 20);
    EXPECT_STREQ(buffer, "player2");
    yini_value_destroy(s_handle_verify);

    bool b_val;
    Yini_ValueHandle b_handle_verify = yini_manager_get_value(verify_manager, "Settings", "fullscreen");
    EXPECT_TRUE(yini_value_get_bool(b_handle_verify, &b_val));
    EXPECT_FALSE(b_val);
    yini_value_destroy(b_handle_verify);

    yini_manager_destroy(verify_manager);
}

TEST(CApiTest, ErrorHandlingIsThreadSafe) {
    const std::string filename = "c_api_thread_test.yini";
    create_test_file(filename, "[TestSection]\nreal_key=123");

    Yini_ManagerHandle manager = yini_manager_create();
    yini_manager_load(manager, filename.c_str());

    auto task = [&](const char* key_name) {
        // Trigger an error by getting a non-existent key
        Yini_ValueHandle handle = yini_manager_get_value(manager, "TestSection", key_name);
        ASSERT_EQ(handle, nullptr);

        // Get the last error
        char error_buffer[256];
        int error_size = yini_manager_get_last_error(manager, error_buffer, sizeof(error_buffer));
        ASSERT_GT(error_size, 0);

        // Verify the error message is the one for this thread
        std::string error_str(error_buffer);
        std::string expected_error_part = "key '" + std::string(key_name) + "'";
        EXPECT_NE(error_str.find(expected_error_part), std::string::npos);
    };

    std::vector<std::thread> threads;
    threads.emplace_back(task, "key1");
    threads.emplace_back(task, "key2");
    threads.emplace_back(task, "key3");
    threads.emplace_back(task, "key4");

    for (auto& t : threads) {
        t.join();
    }

    yini_manager_destroy(manager);
}