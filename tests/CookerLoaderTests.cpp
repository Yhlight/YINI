#include "gtest/gtest.h"
#include "Interop/yini_interop.h"
#include <string>
#include <vector>
#include <cstdlib>
#include <cstdio>

class CookerLoaderTest : public ::testing::Test {
protected:
    const std::string input_yini = "cooker_test.yini";
    const std::string output_ybin = "cooked_test.ybin";

    void SetUp() override {
        // The test yini file is created before the test runs.
        // We just need to clean up any old output files.
        std::remove(output_ybin.c_str());
    }

    void TearDown() override {
        // Clean up the generated file after the test.
        std::remove(output_ybin.c_str());
    }
};

TEST_F(CookerLoaderTest, CookAndLoad) {
    // 1. Cook the .yini file to a .ybin file
    // The test executable runs from `build/tests`, and the CLI is in `build/bin`.
    std::string yini_cli_path = "../bin/yini";
    // The test .yini file is copied by CMake to the same directory as the test executable.
    std::string command = yini_cli_path + " cook -o " + output_ybin + " " + input_yini;

    int result = std::system(command.c_str());
    ASSERT_EQ(result, 0);

    // 2. Load the cooked .ybin file using the interop functions
    void* handle = yini_create_from_file(output_ybin.c_str());
    ASSERT_NE(handle, nullptr) << "yini_create_from_file failed: " << yini_get_last_error();

    // 3. Verify all the values
    int int_val;
    ASSERT_TRUE(yini_get_int(handle, "Test.intValue", &int_val));
    EXPECT_EQ(int_val, 123);

    ASSERT_TRUE(yini_get_int(handle, "Test.negIntValue", &int_val));
    EXPECT_EQ(int_val, -50);

    double large_int_val;
    ASSERT_TRUE(yini_get_double(handle, "Test.largeIntValue", &large_int_val));
    EXPECT_EQ(large_int_val, 2147483648);

    double double_val;
    ASSERT_TRUE(yini_get_double(handle, "Test.doubleValue", &double_val));
    EXPECT_EQ(double_val, 45.67);

    bool bool_val;
    ASSERT_TRUE(yini_get_bool(handle, "Test.boolValue", &bool_val));
    EXPECT_TRUE(bool_val);

    const char* str_val_c = yini_get_string(handle, "Test.stringValue");
    ASSERT_NE(str_val_c, nullptr);
    std::string str_val(str_val_c);
    yini_free_string(str_val_c);
    EXPECT_EQ(str_val, "hello ybin");

    // Test that a macro was resolved during cooking
    double speed_val;
    ASSERT_TRUE(yini_get_double(handle, "Test.speed", &speed_val));
    EXPECT_EQ(speed_val, 10.5);

    // Verify arrays
    int size = yini_get_array_size(handle, "Arrays.int_array");
    ASSERT_EQ(size, 3);
    int item;
    ASSERT_TRUE(yini_get_array_item_as_int(handle, "Arrays.int_array", 0, &item));
    EXPECT_EQ(item, 1);
    ASSERT_TRUE(yini_get_array_item_as_int(handle, "Arrays.int_array", 1, &item));
    EXPECT_EQ(item, 2);
    ASSERT_TRUE(yini_get_array_item_as_int(handle, "Arrays.int_array", 2, &item));
    EXPECT_EQ(item, 3);

    size = yini_get_array_size(handle, "Arrays.string_array");
    ASSERT_EQ(size, 3);
    const char* s_item_c = yini_get_array_item_as_string(handle, "Arrays.string_array", 1);
    ASSERT_NE(s_item_c, nullptr);
    std::string s_item(s_item_c);
    yini_free_string(s_item_c);
    EXPECT_EQ(s_item, "two");


    // Clean up
    yini_destroy(handle);
}
