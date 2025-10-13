#include "Ymeta/YmetaManager.h"
#include "YiniTypes.h"
#include "gtest/gtest.h"

TEST(YmetaManagerTests, GetAndSetValue)
{
    YINI::YmetaManager ymeta_manager;
    ymeta_manager.set_value("my_key", 123.0);
    ASSERT_TRUE(ymeta_manager.has_value("my_key"));
    EXPECT_EQ(std::get<double>(ymeta_manager.get_value("my_key")), 123.0);
}

TEST(YmetaManagerTests, BackupMechanism)
{
    YINI::YmetaManager ymeta_manager;
    for (int i = 0; i < 10; ++i)
    {
        ymeta_manager.set_value("my_key", static_cast<double>(i));
    }

    // The backup mechanism is not directly testable through the public API.
    // This test serves as a placeholder to confirm that repeated `set_value` calls
    // do not cause a crash and that the final value is correct.
    EXPECT_EQ(std::get<double>(ymeta_manager.get_value("my_key")), 9.0);
}

TEST(YmetaManagerTests, SaveAndLoad)
{
    const std::string test_file = "test.yini";
    const std::string ymeta_file = "test.ymeta";

    // Clean up any old test files
    remove(ymeta_file.c_str());

    {
        YINI::YmetaManager ymeta_manager_to_save;
        ymeta_manager_to_save.set_value("my_key", 123.0);
        ymeta_manager_to_save.set_value("my_string", std::string("hello"));
        ymeta_manager_to_save.save(test_file);
    }

    YINI::YmetaManager ymeta_manager_to_load;
    ymeta_manager_to_load.load(test_file);

    ASSERT_TRUE(ymeta_manager_to_load.has_value("my_key"));
    EXPECT_EQ(std::get<double>(ymeta_manager_to_load.get_value("my_key")), 123.0);
    ASSERT_TRUE(ymeta_manager_to_load.has_value("my_string"));
    EXPECT_EQ(std::get<std::string>(ymeta_manager_to_load.get_value("my_string")), "hello");

    // Clean up the test file
    remove(ymeta_file.c_str());
}
