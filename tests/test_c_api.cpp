#include "YINI/Yini.h"
#include <gtest/gtest.h>
#include <string>
#include <vector>

TEST(CAPITest, CreateAndModifyComplexTypes) {
    const char* content = "[MySection]\n";
    char error_buffer[256] = {0};
    YiniDocumentHandle* doc = yini_parse(content, error_buffer, sizeof(error_buffer));
    ASSERT_NE(doc, nullptr);
    ASSERT_EQ(error_buffer[0], '\0');

    // Test Array
    YiniValueHandle* array_val = yini_create_array_value(doc, "MySection", "my_array");
    ASSERT_NE(array_val, nullptr);
    yini_array_add_int(array_val, 123);
    yini_array_add_string(array_val, "hello");
    yini_array_add_bool(array_val, true);

    const YiniSectionHandle* section = yini_get_section_by_name(doc, "MySection");
    ASSERT_NE(section, nullptr);
    const YiniValueHandle* retrieved_array = yini_section_get_value_by_key(section, "my_array");
    ASSERT_NE(retrieved_array, nullptr);
    ASSERT_EQ(yini_value_get_type(retrieved_array), YINI_TYPE_ARRAY);
    ASSERT_EQ(yini_array_get_size(retrieved_array), 3);
    EXPECT_EQ(yini_value_get_int(yini_array_get_value_by_index(retrieved_array, 0)), 123);

    char buffer[20];
    yini_value_get_string(yini_array_get_value_by_index(retrieved_array, 1), buffer, sizeof(buffer));
    EXPECT_STREQ(buffer, "hello");
    EXPECT_EQ(yini_value_get_bool(yini_array_get_value_by_index(retrieved_array, 2)), true);


    // Test Map
    YiniValueHandle* map_val = yini_create_map_value(doc, "MySection", "my_map");
    ASSERT_NE(map_val, nullptr);
    yini_map_set_string(map_val, "map_key", "map_value");
    yini_map_set_int(map_val, "map_int", 456);

    const YiniValueHandle* retrieved_map = yini_section_get_value_by_key(section, "my_map");
    ASSERT_NE(retrieved_map, nullptr);
    ASSERT_EQ(yini_value_get_type(retrieved_map), YINI_TYPE_MAP);
    ASSERT_EQ(yini_map_get_size(retrieved_map), 2);

    const YiniValueHandle* map_str_val = yini_map_get_value_by_key(retrieved_map, "map_key");
    ASSERT_NE(map_str_val, nullptr);
    yini_value_get_string(map_str_val, buffer, sizeof(buffer));
    EXPECT_STREQ(buffer, "map_value");

    const YiniValueHandle* map_int_val = yini_map_get_value_by_key(retrieved_map, "map_int");
    ASSERT_NE(map_int_val, nullptr);
    EXPECT_EQ(yini_value_get_int(map_int_val), 456);

    yini_free_document(doc);
}