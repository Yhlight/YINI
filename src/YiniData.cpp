#include "YINI/YiniData.hpp"
#include <stdexcept>

namespace YINI
{
// Helper for deep copying the variant
YiniVariant deep_copy_variant(const YiniVariant &v)
{
  return std::visit(
      [](const auto &arg) -> YiniVariant
      {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::unique_ptr<YiniArray>>)
        {
          if (!arg)
            return std::unique_ptr<YiniArray>(nullptr);
          auto new_arr = std::make_unique<YiniArray>();
          for (const auto &elem : arg->elements)
          {
            new_arr->elements.push_back(
                elem); // relies on YiniValue's copy constructor
          }
          return new_arr;
        }
        else if constexpr (std::is_same_v<T, std::unique_ptr<YiniList>>)
        {
            if (!arg)
                return std::unique_ptr<YiniList>(nullptr);
            auto new_list = std::make_unique<YiniList>();
            for (const auto& elem : arg->elements)
            {
                new_list->elements.push_back(elem);
            }
            return new_list;
        }
        else if constexpr (std::is_same_v<T, std::unique_ptr<YiniSet>>)
        {
            if (!arg)
                return std::unique_ptr<YiniSet>(nullptr);
            auto new_set = std::make_unique<YiniSet>();
            for (const auto& elem : arg->elements)
            {
                new_set->elements.push_back(elem);
            }
            return new_set;
        }
        else if constexpr (std::is_same_v<T, std::unique_ptr<YiniMap>>)
        {
          if (!arg)
            return std::unique_ptr<YiniMap>(nullptr);
          auto new_map = std::make_unique<YiniMap>();
          for (const auto &[key, val] : arg->elements)
          {
            new_map->elements[key] =
                val; // relies on YiniValue's copy constructor
          }
          return new_map;
        }
        else if constexpr (std::is_same_v<T, std::unique_ptr<YiniDynaValue>>)
        {
          if (!arg)
            return std::unique_ptr<YiniDynaValue>(nullptr);
          auto new_dyna = std::make_unique<YiniDynaValue>();
          new_dyna->value =
              arg->value; // relies on YiniValue's copy constructor
          return new_dyna;
        }
        else if constexpr (std::is_same_v<T, std::unique_ptr<YiniCoord>>)
        {
          if (!arg)
            return std::unique_ptr<YiniCoord>(nullptr);
          return std::make_unique<YiniCoord>(*arg);
        }
        else if constexpr (std::is_same_v<T, std::unique_ptr<YiniColor>>)
        {
          if (!arg)
            return std::unique_ptr<YiniColor>(nullptr);
          return std::make_unique<YiniColor>(*arg);
        }
        else if constexpr (std::is_same_v<T, std::unique_ptr<YiniPath>>)
        {
          if (!arg)
            return std::unique_ptr<YiniPath>(nullptr);
          return std::make_unique<YiniPath>(*arg);
        }
        else
        {
          return arg; // For non-pointer types, copy is fine
        }
      },
      v);
}

YiniValue::YiniValue() = default;
YiniValue::~YiniValue() = default;
YiniValue::YiniValue(YiniValue &&other) noexcept = default;
YiniValue &YiniValue::operator=(YiniValue &&other) noexcept = default;

YiniValue::YiniValue(const YiniValue &other)
    : data(deep_copy_variant(other.data))
{
}

YiniValue &YiniValue::operator=(const YiniValue &other)
{
  if (this != &other)
  {
    data = deep_copy_variant(other.data);
  }
  return *this;
}

void YiniDocument::resolveInheritance()
{
  std::set<std::string> resolved;
  for (auto &section : sections)
  {
    if (resolved.find(section.name) == resolved.end())
    {
      std::vector<std::string> path;
      resolveSectionInheritance(&section, path, resolved);
    }
  }
}

void YiniDocument::resolveSectionInheritance(YiniSection *section,
                                             std::vector<std::string> &path,
                                             std::set<std::string> &resolved)
{
  path.push_back(section->name);

  std::map<std::string, YiniKeyValuePair> merged_pairs;

  // 1. Inherit from parents first
  for (const auto &parent_name : section->inheritedSections)
  {
    // Check for circular dependency
    if (std::find(path.begin(), path.end(), parent_name) != path.end())
    {
      // In a real application, throw a more specific exception
      throw std::runtime_error("Circular inheritance detected: " + parent_name);
    }

    YiniSection *parent_section = findSection(parent_name);
    if (parent_section)
    {
      // Ensure parent is resolved first
      if (resolved.find(parent_name) == resolved.end())
      {
        resolveSectionInheritance(parent_section, path, resolved);
      }

      // Merge parent's pairs. Later parents overwrite earlier ones.
      for (const auto &pair : parent_section->pairs)
      {
        merged_pairs[pair.key] = pair;
      }
    }
  }

  // 2. Apply this section's own pairs, overwriting any inherited ones
  for (const auto &pair : section->pairs)
  {
    merged_pairs[pair.key] = pair;
  }

  // 3. Replace the old pairs with the new merged list
  section->pairs.clear();
  for (const auto &[key, pair] : merged_pairs)
  {
    section->pairs.push_back(pair);
  }

  // Mark as resolved
  resolved.insert(section->name);
  path.pop_back();
}
} // namespace YINI