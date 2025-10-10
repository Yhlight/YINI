#include "gtest/gtest.h"
#include "Interop/yini_interop.h"
#include <string>

TEST(InteropTests, CreateFromFile_NonExistentFile)
{
    // Arrange
    const char* non_existent_file = "non_existent_file.yini";

    // Act
    void* handle = yini_create_from_file(non_existent_file);
    const char* error_message = yini_get_last_error();

    // Assert
    ASSERT_EQ(handle, nullptr);
    ASSERT_NE(error_message, nullptr);
    std::string error_str(error_message);
    ASSERT_NE(error_str.find("Could not open file"), std::string::npos);

    // Cleanup
    yini_destroy(handle);
}
