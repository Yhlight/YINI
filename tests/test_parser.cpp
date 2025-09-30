#include "YINI/Parser.hpp"
#include "YINI/YiniData.hpp"
#include "YINI/YiniException.hpp"
#include <fstream>
#include <gtest/gtest.h>
#include <streambuf>

static std::string read_file_content(const std::string &path)
{
  std::ifstream t(path);
  if (!t.is_open())
    return "";
  std::string str((std::istreambuf_iterator<char>(t)),
                  std::istreambuf_iterator<char>());
  return str;
}

TEST(ParserTest, ParseSimpleSection)
{
  const std::string input = "[TestSection]\nkey = \"value\"";
  YINI::YiniDocument doc;
  YINI::Parser parser(input, doc);
  parser.parse();

  ASSERT_EQ(doc.getSections().size(), 1);
  const auto &section = doc.getSections()[0];
  EXPECT_EQ(section.name, "TestSection");
  ASSERT_EQ(section.pairs.size(), 1);
  const auto &pair = section.pairs[0];
  EXPECT_EQ(pair.key, "key");
  EXPECT_EQ(std::get<std::string>(pair.value.data), "value");
}

TEST(ParserTest, ParseListValue)
{
  const std::string input = R"([Data]
my_list = List(1, "two", true)
)";
  YINI::YiniDocument doc;
  YINI::Parser parser(input, doc);
  parser.parse();

  const auto *section = doc.findSection("Data");
  ASSERT_NE(section, nullptr);
  ASSERT_EQ(section->pairs.size(), 1);
  const auto &pair = section->pairs[0];
  EXPECT_EQ(pair.key, "my_list");

  // Check that it's a list
  auto &list_ptr = std::get<std::unique_ptr<YINI::YiniList>>(pair.value.data);
  ASSERT_NE(list_ptr, nullptr);
  auto &list_elements = list_ptr->elements;
  ASSERT_EQ(list_elements.size(), 3);

  // Check individual elements
  EXPECT_EQ(std::get<int>(list_elements[0].data), 1);
  EXPECT_EQ(std::get<std::string>(list_elements[1].data), "two");
  EXPECT_EQ(std::get<bool>(list_elements[2].data), true);
}

TEST(ParserTest, ParseArrayFromFunctionValue)
{
  const std::string input = R"([Data]
my_array = Array(1, "two", true)
)";
  YINI::YiniDocument doc;
  YINI::Parser parser(input, doc);
  parser.parse();

  const auto *section = doc.findSection("Data");
  ASSERT_NE(section, nullptr);
  ASSERT_EQ(section->pairs.size(), 1);
  const auto &pair = section->pairs[0];
  EXPECT_EQ(pair.key, "my_array");

  // Check that it's an array
  auto &array_ptr = std::get<std::unique_ptr<YINI::YiniArray>>(pair.value.data);
  ASSERT_NE(array_ptr, nullptr);
  auto &array_elements = array_ptr->elements;
  ASSERT_EQ(array_elements.size(), 3);

  // Check individual elements
  EXPECT_EQ(std::get<int>(array_elements[0].data), 1);
  EXPECT_EQ(std::get<std::string>(array_elements[1].data), "two");
  EXPECT_EQ(std::get<bool>(array_elements[2].data), true);
}

TEST(ParserTest, ParseSetValue)
{
  const std::string input = R"([Data]
my_set = Set(1, "two", 1, true, "two")
)";
  YINI::YiniDocument doc;
  YINI::Parser parser(input, doc);
  parser.parse();

  const auto *section = doc.findSection("Data");
  ASSERT_NE(section, nullptr);
  ASSERT_EQ(section->pairs.size(), 1);
  const auto &pair = section->pairs[0];
  EXPECT_EQ(pair.key, "my_set");

  // Check that it's a set and uniqueness is enforced for simple types
  auto &set_ptr = std::get<std::unique_ptr<YINI::YiniSet>>(pair.value.data);
  ASSERT_NE(set_ptr, nullptr);
  auto &set_elements = set_ptr->elements;
  ASSERT_EQ(set_elements.size(), 3);

  // Check individual elements (order is preserved, but duplicates are removed)
  EXPECT_EQ(std::get<int>(set_elements[0].data), 1);
  EXPECT_EQ(std::get<std::string>(set_elements[1].data), "two");
  EXPECT_EQ(std::get<bool>(set_elements[2].data), true);
}

TEST(ParserTest, ParseMapValue)
{
  const std::string input = R"([Data]
my_map = {
    "key1": "value1",
    key2: 123,
    "key3": true,
    key4: [1, "two"]
}
)";
  YINI::YiniDocument doc;
  YINI::Parser parser(input, doc);
  parser.parse();

  const auto *section = doc.findSection("Data");
  ASSERT_NE(section, nullptr);
  ASSERT_EQ(section->pairs.size(), 1);
  const auto &pair = section->pairs[0];
  EXPECT_EQ(pair.key, "my_map");

  // Check that it's a map
  auto &map_ptr = std::get<std::unique_ptr<YINI::YiniMap>>(pair.value.data);
  ASSERT_NE(map_ptr, nullptr);
  auto &map_elements = map_ptr->elements;
  ASSERT_EQ(map_elements.size(), 4);

  // Check individual elements
  EXPECT_EQ(std::get<std::string>(map_elements["key1"].data), "value1");
  EXPECT_EQ(std::get<int>(map_elements["key2"].data), 123);
  EXPECT_EQ(std::get<bool>(map_elements["key3"].data), true);

  // Check nested array
  auto &nested_array_ptr =
      std::get<std::unique_ptr<YINI::YiniArray>>(map_elements["key4"].data);
  ASSERT_NE(nested_array_ptr, nullptr);
  auto &nested_array = nested_array_ptr->elements;
  ASSERT_EQ(nested_array.size(), 2);
  EXPECT_EQ(std::get<int>(nested_array[0].data), 1);
  EXPECT_EQ(std::get<std::string>(nested_array[1].data), "two");
}

TEST(ParserTest, ParseCustomValueTypes)
{
  const std::string input = R"([CustomTypes]
pos2d = Coord(1.5, 2.5)
pos3d = Coord(1, 2, 3)
color_hex = #FF00FF
color_func = Color(255, 128, 0)
asset_path = Path(characters/player.fbx)
)";
  YINI::YiniDocument doc;
  YINI::Parser parser(input, doc);
  parser.parse();

  const auto *section = doc.findSection("CustomTypes");
  ASSERT_NE(section, nullptr);

  // Test Coord(2D)
  auto p1_it = std::find_if(section->pairs.begin(), section->pairs.end(),
                            [](const auto &p) { return p.key == "pos2d"; });
  ASSERT_NE(p1_it, section->pairs.end());
  auto &coord2d_ptr =
      std::get<std::unique_ptr<YINI::YiniCoord>>(p1_it->value.data);
  ASSERT_NE(coord2d_ptr, nullptr);
  EXPECT_FALSE(coord2d_ptr->is_3d);
  EXPECT_EQ(coord2d_ptr->x, 1.5);
  EXPECT_EQ(coord2d_ptr->y, 2.5);

  // Test Coord(3D)
  auto p2_it = std::find_if(section->pairs.begin(), section->pairs.end(),
                            [](const auto &p) { return p.key == "pos3d"; });
  ASSERT_NE(p2_it, section->pairs.end());
  auto &coord3d_ptr =
      std::get<std::unique_ptr<YINI::YiniCoord>>(p2_it->value.data);
  ASSERT_NE(coord3d_ptr, nullptr);
  EXPECT_TRUE(coord3d_ptr->is_3d);
  EXPECT_EQ(coord3d_ptr->x, 1);
  EXPECT_EQ(coord3d_ptr->y, 2);
  EXPECT_EQ(coord3d_ptr->z, 3);

  // Test Color (Hex)
  auto p3_it = std::find_if(section->pairs.begin(), section->pairs.end(),
                            [](const auto &p) { return p.key == "color_hex"; });
  ASSERT_NE(p3_it, section->pairs.end());
  auto &color_hex_ptr =
      std::get<std::unique_ptr<YINI::YiniColor>>(p3_it->value.data);
  ASSERT_NE(color_hex_ptr, nullptr);
  EXPECT_EQ(color_hex_ptr->r, 255);
  EXPECT_EQ(color_hex_ptr->g, 0);
  EXPECT_EQ(color_hex_ptr->b, 255);

  // Test Color (Func)
  auto p4_it =
      std::find_if(section->pairs.begin(), section->pairs.end(),
                   [](const auto &p) { return p.key == "color_func"; });
  ASSERT_NE(p4_it, section->pairs.end());
  auto &color_func_ptr =
      std::get<std::unique_ptr<YINI::YiniColor>>(p4_it->value.data);
  ASSERT_NE(color_func_ptr, nullptr);
  EXPECT_EQ(color_func_ptr->r, 255);
  EXPECT_EQ(color_func_ptr->g, 128);
  EXPECT_EQ(color_func_ptr->b, 0);

  // Test Path
  auto p5_it =
      std::find_if(section->pairs.begin(), section->pairs.end(),
                   [](const auto &p) { return p.key == "asset_path"; });
  ASSERT_NE(p5_it, section->pairs.end());
  auto &path_ptr = std::get<std::unique_ptr<YINI::YiniPath>>(p5_it->value.data);
  ASSERT_NE(path_ptr, nullptr);
  EXPECT_EQ(path_ptr->path_value, "characters/player.fbx");
}

TEST(ParserTest, ParseDynaValue)
{
  const std::string input = "[Config]\nkey = Dyna(1)";
  YINI::YiniDocument doc;
  YINI::Parser parser(input, doc);
  parser.parse();

  const auto *config_section = doc.findSection("Config");
  ASSERT_NE(config_section, nullptr);
  ASSERT_EQ(config_section->pairs.size(), 1);

  const auto &pair = config_section->pairs[0];
  EXPECT_EQ(pair.key, "key");

  auto &dyna_ptr =
      std::get<std::unique_ptr<YINI::YiniDynaValue>>(pair.value.data);
  ASSERT_NE(dyna_ptr, nullptr);
  EXPECT_EQ(std::get<int>(dyna_ptr->value.data), 1);
}

TEST(ParserTest, ThrowOnUnclosedSection)
{
  const std::string input = "[TestSection";
  YINI::YiniDocument doc;
  YINI::Parser parser(input, doc);

  try
  {
    parser.parse();
    FAIL() << "Expected YINI::YiniException";
  }
  catch (const YINI::YiniException &e)
  {
    EXPECT_EQ(e.getLine(), 1);
    EXPECT_EQ(e.getColumn(), 13);
    EXPECT_STREQ("Expected ']' to close section header.", e.what());
  }
  catch (...)
  {
    FAIL() << "Expected YINI::YiniException";
  }
}

TEST(ParserTest, ParseArithmetic)
{
  const std::string input = "[#define]\n"
                            "base_val = 10\n"
                            "factor = 2\n"
                            "[Data]\n"
                            "val1 = 5 + @base_val\n"
                            "val2 = @base_val * (3 + @factor)\n"
                            "val3 = 100 / 4 - 5\n"
                            "val4 = 3.5 * 2\n";
  YINI::YiniDocument doc;
  YINI::Parser parser(input, doc);
  parser.parse();

  const auto *data_section = doc.findSection("Data");
  ASSERT_NE(data_section, nullptr);

  auto p1_it =
      std::find_if(data_section->pairs.begin(), data_section->pairs.end(),
                   [](const auto &p) { return p.key == "val1"; });
  ASSERT_NE(p1_it, data_section->pairs.end());
  EXPECT_EQ(std::get<int>(p1_it->value.data), 15);

  auto p2_it =
      std::find_if(data_section->pairs.begin(), data_section->pairs.end(),
                   [](const auto &p) { return p.key == "val2"; });
  ASSERT_NE(p2_it, data_section->pairs.end());
  EXPECT_EQ(std::get<int>(p2_it->value.data), 50);

  auto p3_it =
      std::find_if(data_section->pairs.begin(), data_section->pairs.end(),
                   [](const auto &p) { return p.key == "val3"; });
  ASSERT_NE(p3_it, data_section->pairs.end());
  EXPECT_EQ(std::get<int>(p3_it->value.data), 20);

  auto p4_it =
      std::find_if(data_section->pairs.begin(), data_section->pairs.end(),
                   [](const auto &p) { return p.key == "val4"; });
  ASSERT_NE(p4_it, data_section->pairs.end());
  EXPECT_EQ(std::get<double>(p4_it->value.data), 7.0);
}

TEST(ParserTest, ParseFileIncludes)
{
  const std::string file_path = std::string(TEST_DATA_DIR) + "/include_test.yini";
  const std::string input = read_file_content(file_path);
  ASSERT_FALSE(input.empty());

  YINI::YiniDocument doc;
  YINI::Parser parser(input, doc, TEST_DATA_DIR);
  parser.parse();

  ASSERT_EQ(doc.getSections().size(), 3); // Shared, BaseOnly, MainOnly

  YINI::YiniValue val;
  ASSERT_TRUE(doc.getDefine("base_macro", val));
  EXPECT_EQ(std::get<std::string>(val.data), "base");

  const auto *shared_section = doc.findSection("Shared");
  ASSERT_NE(shared_section, nullptr);
  ASSERT_EQ(shared_section->pairs.size(), 3);

  auto key1_it =
      std::find_if(shared_section->pairs.begin(), shared_section->pairs.end(),
                   [](const auto &p) { return p.key == "key1"; });
  ASSERT_NE(key1_it, shared_section->pairs.end());
  EXPECT_EQ(std::get<std::string>(key1_it->value.data), "from_base");

  auto key2_it =
      std::find_if(shared_section->pairs.begin(), shared_section->pairs.end(),
                   [](const auto &p) { return p.key == "key2"; });
  ASSERT_NE(key2_it, shared_section->pairs.end());
  EXPECT_EQ(std::get<std::string>(key2_it->value.data), "overridden");

  auto key3_it =
      std::find_if(shared_section->pairs.begin(), shared_section->pairs.end(),
                   [](const auto &p) { return p.key == "key3"; });
  ASSERT_NE(key3_it, shared_section->pairs.end());
  EXPECT_EQ(std::get<std::string>(key3_it->value.data), "from_main");

  ASSERT_NE(doc.findSection("BaseOnly"), nullptr);

  const auto *main_only_section = doc.findSection("MainOnly");
  ASSERT_NE(main_only_section, nullptr);
  ASSERT_EQ(main_only_section->pairs.size(), 2);
  auto ref_it = std::find_if(main_only_section->pairs.begin(),
                             main_only_section->pairs.end(),
                             [](const auto &p) { return p.key == "ref"; });
  ASSERT_NE(ref_it, main_only_section->pairs.end());
  EXPECT_EQ(std::get<std::string>(ref_it->value.data), "base");
}

TEST(ParserTest, ParseValueTypes)
{
  const std::string input = "[DataTypes]\n"
                            "integer = 123\n"
                            "float = 3.14\n"
                            "boolean_true = true\n"
                            "boolean_false = false\n";
  YINI::YiniDocument doc;
  YINI::Parser parser(input, doc);
  parser.parse();

  ASSERT_EQ(doc.getSections().size(), 1);
  const auto &section = doc.getSections()[0];
  EXPECT_EQ(section.name, "DataTypes");
  ASSERT_EQ(section.pairs.size(), 4);

  const auto &int_pair = section.pairs[0];
  EXPECT_EQ(int_pair.key, "integer");
  EXPECT_EQ(std::get<int>(int_pair.value.data), 123);

  const auto &float_pair = section.pairs[1];
  EXPECT_EQ(float_pair.key, "float");
  EXPECT_EQ(std::get<double>(float_pair.value.data), 3.14);

  const auto &bool_true_pair = section.pairs[2];
  EXPECT_EQ(bool_true_pair.key, "boolean_true");
  EXPECT_EQ(std::get<bool>(bool_true_pair.value.data), true);

  const auto &bool_false_pair = section.pairs[3];
  EXPECT_EQ(bool_false_pair.key, "boolean_false");
  EXPECT_EQ(std::get<bool>(bool_false_pair.value.data), false);
}

TEST(ParserTest, ParseArrayValue)
{
  const std::string input = "[Arrays]\n"
                            "int_array = [1, 2, 3]\n";
  YINI::YiniDocument doc;
  YINI::Parser parser(input, doc);
  parser.parse();

  ASSERT_EQ(doc.getSections().size(), 1);
  const auto &section = doc.getSections()[0];
  ASSERT_EQ(section.pairs.size(), 1);
  const auto &pair = section.pairs[0];
  EXPECT_EQ(pair.key, "int_array");

  auto &arr_ptr = std::get<std::unique_ptr<YINI::YiniArray>>(pair.value.data);
  ASSERT_NE(arr_ptr, nullptr);
  auto &arr = arr_ptr->elements;
  ASSERT_EQ(arr.size(), 3);
  EXPECT_EQ(std::get<int>(arr[0].data), 1);
  EXPECT_EQ(std::get<int>(arr[1].data), 2);
  EXPECT_EQ(std::get<int>(arr[2].data), 3);
}

TEST(ParserTest, ParseSectionInheritance)
{
  const std::string input = "[Derived : Base1, Base2]";
  YINI::YiniDocument doc;
  YINI::Parser parser(input, doc);
  parser.parse();

  ASSERT_EQ(doc.getSections().size(), 1);
  const auto &section = doc.getSections()[0];
  EXPECT_EQ(section.name, "Derived");
  ASSERT_EQ(section.inheritedSections.size(), 2);
  EXPECT_EQ(section.inheritedSections[0], "Base1");
  EXPECT_EQ(section.inheritedSections[1], "Base2");
}

TEST(ParserTest, ParseQuickRegistration)
{
  const std::string input = "[Registry]\n"
                            "+= 1\n"
                            "+= \"two\"\n"
                            "+= true\n";
  YINI::YiniDocument doc;
  YINI::Parser parser(input, doc);
  parser.parse();

  ASSERT_EQ(doc.getSections().size(), 1);
  const auto &section = doc.getSections()[0];
  EXPECT_EQ(section.name, "Registry");
  ASSERT_EQ(section.registrationList.size(), 3);
  EXPECT_EQ(std::get<int>(section.registrationList[0].data), 1);
  EXPECT_EQ(std::get<std::string>(section.registrationList[1].data), "two");
  EXPECT_EQ(std::get<bool>(section.registrationList[2].data), true);
}

TEST(ParserTest, ParseMacros)
{
  const std::string input = "[#define]\n"
                            "name = \"YINI\"\n"
                            "[UI]\n"
                            "UIName = @name\n";
  YINI::YiniDocument doc;
  YINI::Parser parser(input, doc);
  parser.parse();

  ASSERT_EQ(doc.getSections().size(), 1);

  const auto *ui_section = doc.findSection("UI");
  ASSERT_NE(ui_section, nullptr);
  ASSERT_EQ(ui_section->pairs.size(), 1);
  const auto &pair = ui_section->pairs[0];
  EXPECT_EQ(pair.key, "UIName");
  EXPECT_EQ(std::get<std::string>(pair.value.data), "YINI");
}

TEST(ParserTest, ParseSectionWithComments)
{
  const std::string input = "// This is a whole line comment\n"
                            "[TestSection] // This is an inline comment\n"
                            "key = \"value\"\n";
  YINI::YiniDocument doc;
  YINI::Parser parser(input, doc);
  parser.parse();

  ASSERT_EQ(doc.getSections().size(), 1);
  const auto &section = doc.getSections()[0];
  EXPECT_EQ(section.name, "TestSection");
  ASSERT_EQ(section.pairs.size(), 1);
  const auto &pair = section.pairs[0];
  EXPECT_EQ(pair.key, "key");
  EXPECT_EQ(std::get<std::string>(pair.value.data), "value");
}