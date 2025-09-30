#include "YINI/YiniData.hpp"
#include "YINI/YiniException.hpp"
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
        else if constexpr (std::is_same_v<T, std::unique_ptr<YiniPair>>)
        {
          if (!arg)
            return std::unique_ptr<YiniPair>(nullptr);
          auto new_pair = std::make_unique<YiniPair>();
          new_pair->key = arg->key;
          new_pair->value = arg->value; // relies on YiniValue's copy constructor
          return new_pair;
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

bool operator==(const YiniValue &lhs, const YiniValue &rhs)
{
  // If the types are different, they are not equal.
  if (lhs.data.index() != rhs.data.index())
  {
    return false;
  }

  // Visit both variants to compare their contents.
  return std::visit(
      [](const auto &lhs_val, const auto &rhs_val) -> bool
      {
        // This generic lambda is instantiated for all pairs of types.
        // We only want to compare if the types are identical.
        if constexpr (std::is_same_v<std::decay_t<decltype(lhs_val)>,
                                     std::decay_t<decltype(rhs_val)>>)
        {
          using T = std::decay_t<decltype(lhs_val)>;

          // For unique_ptr types, we need to handle null pointers and
          // dereference.
          if constexpr (std::is_same_v<T, std::unique_ptr<YiniArray>> ||
                        std::is_same_v<T, std::unique_ptr<YiniList>> ||
                        std::is_same_v<T, std::unique_ptr<YiniSet>> ||
                        std::is_same_v<T, std::unique_ptr<YiniMap>> ||
                        std::is_same_v<T, std::unique_ptr<YiniPair>> ||
                        std::is_same_v<T, std::unique_ptr<YiniDynaValue>> ||
                        std::is_same_v<T, std::unique_ptr<YiniCoord>> ||
                        std::is_same_v<T, std::unique_ptr<YiniColor>> ||
                        std::is_same_v<T, std::unique_ptr<YiniPath>>)
          {
            if (lhs_val && rhs_val)
            {
              return *lhs_val == *rhs_val; // Use the struct's operator==
            }
            return !lhs_val && !rhs_val; // Both are null is considered equal.
          }
          else
          {
            // For primitive types, direct comparison is fine.
            return lhs_val == rhs_val;
          }
        }
        else
        {
          // This branch will be taken for non-matching types.
          // The index check above ensures this is dead code at runtime,
          // but it's needed for compilation.
          return false;
        }
      },
      lhs.data, rhs.data);
}

void YiniDocument::resolveInheritance()
{
  std::lock_guard<std::mutex> lock(docMutex);
  std::set<std::string> resolved;
  for (auto &section : sectionList)
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
      throw LogicException("Circular inheritance detected: " + parent_name, 0,
                           0);
    }

    YiniSection *parent_section = findSectionInternal(parent_name);
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

// --- Rule of Five Implementation for YiniDocument ---

YiniDocument::YiniDocument(const YiniDocument &other)
{
  std::lock_guard<std::mutex> lock(other.docMutex);
  sectionList = other.sectionList;
  defineMap = other.defineMap;
}

YiniDocument &YiniDocument::operator=(const YiniDocument &other)
{
  if (this != &other)
  {
    // Lock both mutexes without deadlock
    std::lock(docMutex, other.docMutex);
    std::lock_guard<std::mutex> this_lock(docMutex, std::adopt_lock);
    std::lock_guard<std::mutex> other_lock(other.docMutex, std::adopt_lock);

    sectionList = other.sectionList;
    defineMap = other.defineMap;
  }
  return *this;
}

YiniDocument::YiniDocument(YiniDocument &&other) noexcept
{
  std::lock_guard<std::mutex> lock(other.docMutex);
  sectionList = std::move(other.sectionList);
  defineMap = std::move(other.defineMap);
}

YiniDocument &YiniDocument::operator=(YiniDocument &&other) noexcept
{
  if (this != &other)
  {
    std::lock(docMutex, other.docMutex);
    std::lock_guard<std::mutex> this_lock(docMutex, std::adopt_lock);
    std::lock_guard<std::mutex> other_lock(other.docMutex, std::adopt_lock);

    sectionList = std::move(other.sectionList);
    defineMap = std::move(other.defineMap);
  }
  return *this;
}

} // namespace YINI