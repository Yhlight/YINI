#include "YINI/Yini.h"
#include <gtest/gtest.h>

TEST(CApiTest, ExposeMapsMacrosAndHandleErrors) {
    const char* yini_content = R"yini(
[#define]
my_macro = "hello world"
another_macro = 123

[MySection]
my_map = {
    key1: "value1",
    key2: 42,
    key3: true
}
+= "registration_value_1"
+= 100
)yini";

    YiniDocumentHandle* doc = yini_parse(yini_content, nullptr, 0);
    ASSERT_NE(doc, nullptr);

    // Test Macro API
    EXPECT_EQ(yini_get_define_count(doc), 2);

    // Test macros by key for robustness
    const YiniValueHandle* macro_val;
    char value_buffer[64];
    int int_val;
    bool bool_val;

    macro_val = yini_get_define_by_key(doc, "my_macro");
    ASSERT_NE(macro_val, nullptr);
    EXPECT_EQ(yini_value_get_type(macro_val), YINI_TYPE_STRING);
    yini_value_get_string(macro_val, value_buffer, sizeof(value_buffer));
    EXPECT_STREQ(value_buffer, "hello world");

    macro_val = yini_get_define_by_key(doc, "another_macro");
    ASSERT_NE(macro_val, nullptr);
    EXPECT_EQ(yini_value_get_type(macro_val), YINI_TYPE_INT);
    ASSERT_TRUE(yini_value_get_int(macro_val, &int_val));
    EXPECT_EQ(int_val, 123);
    // Test for type mismatch error handling
    ASSERT_FALSE(yini_value_get_bool(macro_val, &bool_val));


    // Test Section and Map API
    const YiniSectionHandle* section = yini_get_section_by_name(doc, "MySection");
    ASSERT_NE(section, nullptr);

    const YiniValueHandle* map_value = yini_section_get_value_by_key(section, "my_map");
    ASSERT_NE(map_value, nullptr);
    EXPECT_EQ(yini_value_get_type(map_value), YINI_TYPE_MAP);

    EXPECT_EQ(yini_map_get_size(map_value), 3);

    // Test map value access by key
    const YiniValueHandle* map_item_int = yini_map_get_value_by_key(map_value, "key2");
    ASSERT_NE(map_item_int, nullptr);
    EXPECT_EQ(yini_value_get_type(map_item_int), YINI_TYPE_INT);
    ASSERT_TRUE(yini_value_get_int(map_item_int, &int_val));
    EXPECT_EQ(int_val, 42);

    const YiniValueHandle* map_item_bool = yini_map_get_value_by_key(map_value, "key3");
    ASSERT_NE(map_item_bool, nullptr);
    EXPECT_EQ(yini_value_get_type(map_item_bool), YINI_TYPE_BOOL);
    ASSERT_TRUE(yini_value_get_bool(map_item_bool, &bool_val));
    EXPECT_TRUE(bool_val);

    // Test map key access by index
    char key_buffer[64];
    yini_map_get_key_by_index(map_value, 1, key_buffer, sizeof(key_buffer));
    EXPECT_STREQ(key_buffer, "key2");

    // Test value-to-string conversion
    char string_val_buffer[256];
    yini_value_to_string(map_value, string_val_buffer, sizeof(string_val_buffer));
    // Note: std::map sorts keys alphabetically.
    const char* expected_map_str = "{\"key1\": \"value1\", \"key2\": 42, \"key3\": true}";
    EXPECT_STREQ(string_val_buffer, expected_map_str);


    // Test Quick Registration API
    EXPECT_EQ(yini_section_get_registration_count(section), 2);
    const YiniValueHandle* reg_val = yini_section_get_registered_value_by_index(section, 0);
    ASSERT_NE(reg_val, nullptr);
    EXPECT_EQ(yini_value_get_type(reg_val), YINI_TYPE_STRING);
    yini_value_get_string(reg_val, value_buffer, sizeof(value_buffer));
    EXPECT_STREQ(value_buffer, "registration_value_1");

    reg_val = yini_section_get_registered_value_by_index(section, 1);
    ASSERT_NE(reg_val, nullptr);
    EXPECT_EQ(yini_value_get_type(reg_val), YINI_TYPE_INT);
    ASSERT_TRUE(yini_value_get_int(reg_val, &int_val));
    EXPECT_EQ(int_val, 100);


    yini_free_document(doc);
}