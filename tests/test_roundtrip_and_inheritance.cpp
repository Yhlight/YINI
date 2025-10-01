#include "YINI/JsonDeserializer.hpp"
#include "YINI/JsonSerializer.hpp"
#include "YINI/Parser.hpp"
#include "YINI/YiniManager.hpp"
#include <gtest/gtest.h>
#include <variant>

// Helper to compare YiniValue objects for equality.
// This is a simplified comparison for testing purposes.
bool areYiniValuesEqual(const YINI::YiniValue &v1, const YINI::YiniValue &v2)
{
  if (v1.data.index() != v2.data.index())
    return false;

  if (std::holds_alternative<std::string>(v1.data))
  {
    return std::get<std::string>(v1.data) == std::get<std::string>(v2.data);
  }
  if (std::holds_alternative<int>(v1.data))
  {
    return std::get<int>(v1.data) == std::get<int>(v2.data);
  }
  if (std::holds_alternative<double>(v1.data))
  {
    return std::abs(std::get<double>(v1.data) - std::get<double>(v2.data)) <
           1e-9;
  }
  if (std::holds_alternative<bool>(v1.data))
  {
    return std::get<bool>(v1.data) == std::get<bool>(v2.data);
  }
  // Note: This helper doesn't recursively compare complex types like
  // arrays/maps for simplicity. The test will rely on the fact that if
  // serialization and deserialization work, the structure will be preserved. A
  // full deep comparison would be more robust but also more complex.
  return true;
}

TEST(RoundtripTest, FullSerializationDeserialization)
{
  const std::string input = R"(
[#define]
version = 1.2

[Base]
base_value = 100

[Core : Base]
name = "YINI"
version = @version
enabled = true
data = [1, 2, "simple_string_in_array"]
my_list = List(10, "foo")
my_set = (1, "bar", 1)
color = #FF00FF

+= "registered_value"
)";

  // 1. Parse original document
  YINI::YiniDocument original_doc;
  YINI::Parser parser(input, original_doc, ".");
  parser.parse();
  original_doc.resolveInheritance();

  // 2. Serialize to JSON
  std::string json_output = YINI::JsonSerializer::serialize(original_doc);

  // 3. Deserialize back to a new document
  YINI::YiniDocument new_doc;
  bool success = YINI::JsonDeserializer::deserialize(json_output, new_doc);
  ASSERT_TRUE(success);

  // 4. Compare the documents
  // Check defines
  YINI::YiniValue original_version, new_version;
  ASSERT_TRUE(original_doc.getDefine("version", original_version));
  ASSERT_TRUE(new_doc.getDefine("version", new_version));
  EXPECT_TRUE(areYiniValuesEqual(original_version, new_version));

  // Check sections
  auto *original_core = original_doc.findSection("Core");
  auto *new_core = new_doc.findSection("Core");
  ASSERT_NE(original_core, nullptr);
  ASSERT_NE(new_core, nullptr);

  // Check inherited value
  auto it_base =
      std::find_if(new_core->pairs.begin(), new_core->pairs.end(),
                   [](const auto &p) { return p.key == "base_value"; });
  ASSERT_NE(it_base, new_core->pairs.end());
  ASSERT_TRUE(std::holds_alternative<int>(it_base->value.data));
  EXPECT_EQ(std::get<int>(it_base->value.data), 100);

  // Check list value
  auto it_list =
      std::find_if(new_core->pairs.begin(), new_core->pairs.end(),
                   [](const auto &p) { return p.key == "my_list"; });
  ASSERT_NE(it_list, new_core->pairs.end());
  ASSERT_TRUE(std::holds_alternative<std::unique_ptr<YINI::YiniList>>(it_list->value.data));
  const auto& list_ptr = std::get<std::unique_ptr<YINI::YiniList>>(it_list->value.data);
  ASSERT_NE(list_ptr, nullptr);
  ASSERT_EQ(list_ptr->elements.size(), 2);
  EXPECT_EQ(std::get<int>(list_ptr->elements[0].data), 10);
  EXPECT_EQ(std::get<std::string>(list_ptr->elements[1].data), "foo");

  // Check set value
  auto it_set =
      std::find_if(new_core->pairs.begin(), new_core->pairs.end(),
                   [](const auto &p) { return p.key == "my_set"; });
  ASSERT_NE(it_set, new_core->pairs.end());
  ASSERT_TRUE(std::holds_alternative<std::unique_ptr<YINI::YiniSet>>(it_set->value.data));
  const auto& set_ptr = std::get<std::unique_ptr<YINI::YiniSet>>(it_set->value.data);
  ASSERT_NE(set_ptr, nullptr);
  ASSERT_EQ(set_ptr->elements.size(), 2); // Uniqueness enforced
  EXPECT_EQ(std::get<int>(set_ptr->elements[0].data), 1);
  EXPECT_EQ(std::get<std::string>(set_ptr->elements[1].data), "bar");

  // Check registration list
  ASSERT_EQ(new_core->registrationList.size(), 1);
  ASSERT_TRUE(
      std::holds_alternative<std::string>(new_core->registrationList[0].data));
  EXPECT_EQ(std::get<std::string>(new_core->registrationList[0].data),
            "registered_value");
}

TEST(InheritanceTest, MergingAndOverriding)
{
  const std::string input = R"(
[Parent1]
val1 = 1
val2 = "original"

[Parent2]
val2 = "override"
val3 = true

[Child : Parent1, Parent2]
val1 = 10
val4 = 3.14
)";

  YINI::YiniDocument doc;
  YINI::Parser parser(input, doc, ".");
  parser.parse();
  doc.resolveInheritance();

  auto *child_section = doc.findSection("Child");
  ASSERT_NE(child_section, nullptr);

  // Check that the parser correctly identified the parents before resolution
  ASSERT_EQ(child_section->inheritedSections.size(), 2);
  EXPECT_EQ(child_section->inheritedSections[0], "Parent1");
  EXPECT_EQ(child_section->inheritedSections[1], "Parent2");

  // Create a map of the final key-value pairs for easy lookup
  std::map<std::string, YINI::YiniValue> pairs_map;
  for (const auto &p : child_section->pairs)
  {
    pairs_map[p.key] = p.value;
  }

  // Check that there are 4 pairs total
  ASSERT_EQ(pairs_map.size(), 4);

  // Test for val1 (overridden by Child)
  ASSERT_TRUE(std::holds_alternative<int>(pairs_map["val1"].data));
  EXPECT_EQ(std::get<int>(pairs_map["val1"].data), 10);

  // Test for val2 (overridden by Parent2)
  ASSERT_TRUE(std::holds_alternative<std::string>(pairs_map["val2"].data));
  EXPECT_EQ(std::get<std::string>(pairs_map["val2"].data), "override");

  // Test for val3 (inherited from Parent2)
  ASSERT_TRUE(std::holds_alternative<bool>(pairs_map["val3"].data));
  EXPECT_EQ(std::get<bool>(pairs_map["val3"].data), true);

  // Test for val4 (defined in Child)
  ASSERT_TRUE(std::holds_alternative<double>(pairs_map["val4"].data));
  EXPECT_EQ(std::get<double>(pairs_map["val4"].data), 3.14);
}