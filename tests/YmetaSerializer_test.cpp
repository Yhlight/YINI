#include <gtest/gtest.h>
#include "YmetaSerializer.h"
#include "YiniData.h"
#include <filesystem>

TEST(YmetaSerializerTest, SaveAndLoad)
{
    Yini::YiniData original_data;
    original_data.addMacro("my_macro", Yini::YiniValue(Yini::YiniVariant("macro_value")));

    Yini::YiniSection section1("Section1");
    section1.addKeyValuePair("key1", Yini::YiniValue(Yini::YiniVariant(123)));
    section1.addValue(Yini::YiniValue(Yini::YiniVariant("val1")));
    original_data.addSection(section1);

    Yini::YmetaSerializer serializer;
    std::string test_file = "test.ymeta";
    ASSERT_TRUE(serializer.save(original_data, test_file));

    Yini::YiniData loaded_data = serializer.load(test_file);

    // Compare macros
    ASSERT_EQ(original_data.getMacros().size(), loaded_data.getMacros().size());
    EXPECT_EQ(original_data.getMacros().at("my_macro").get<Yini::YiniString>(), loaded_data.getMacros().at("my_macro").get<Yini::YiniString>());

    // Compare sections
    ASSERT_EQ(original_data.getSections().size(), loaded_data.getSections().size());
    const auto* loaded_section1 = loaded_data.getSection("Section1");
    ASSERT_NE(loaded_section1, nullptr);
    EXPECT_EQ(section1.getKeyValues().at("key1").get<Yini::YiniInteger>(), loaded_section1->getKeyValues().at("key1").get<Yini::YiniInteger>());
    EXPECT_EQ(section1.getValues()[0].get<Yini::YiniString>(), loaded_section1->getValues()[0].get<Yini::YiniString>());

    std::filesystem::remove(test_file);
}
