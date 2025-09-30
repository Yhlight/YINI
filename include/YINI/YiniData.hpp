/**
 * @file YiniData.hpp
 * @brief Defines the core data structures for the YINI object model.
 *
 * This file contains the definitions for all value types, sections, and the
 * main document class that represent a parsed YINI file in memory.
 */

#ifndef YINI_DATA_HPP
#define YINI_DATA_HPP

#include <algorithm>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <variant>
#include <vector>
#include <mutex>

namespace YINI
{
// Forward declarations
struct YiniValue;
struct YiniArray;
struct YiniList;
struct YiniSet;
struct YiniMap;
struct YiniPair;
struct YiniDynaValue;
struct YiniCoord;
struct YiniColor;
struct YiniPath;

/**
 * @brief A variant type that holds any of the possible YINI data types.
 * @note Complex, recursive types are held by unique_ptr to break circular
 * dependencies.
 */
using YiniVariant =
    std::variant<std::string, int, double, bool, std::unique_ptr<YiniArray>,
                 std::unique_ptr<YiniList>, std::unique_ptr<YiniSet>,
                 std::unique_ptr<YiniMap>, std::unique_ptr<YiniPair>,
                 std::unique_ptr<YiniDynaValue>, std::unique_ptr<YiniCoord>,
                 std::unique_ptr<YiniColor>, std::unique_ptr<YiniPath>>;

/**
 * @brief The primary discriminated union for all values in the YINI model.
 * @details This struct is a wrapper around the YiniVariant, enabling recursive
 *          data structures like arrays of arrays. It manages the lifetime of
 *          heap-allocated complex types through its copy/move constructors
 *          and assignment operators.
 */
struct YiniValue
{
  YiniVariant data;

  // Constructors
  YiniValue();
  YiniValue(const YiniValue &other);
  YiniValue(YiniValue &&other) noexcept;

  // Assignment operators
  YiniValue &operator=(const YiniValue &other);
  YiniValue &operator=(YiniValue &&other) noexcept;

  // Destructor
  ~YiniValue();
};

// Equality operators for comprehensive value comparison
bool operator==(const YiniValue &lhs, const YiniValue &rhs);
inline bool operator!=(const YiniValue &lhs, const YiniValue &rhs) { return !(lhs == rhs); }


/**
 * @brief Represents an array of YiniValue elements, created with `[...]` or
 * `Array(...)`.
 */
struct YiniArray
{
  std::vector<YiniValue> elements;
  bool operator==(const YiniArray& other) const { return elements == other.elements; }
};

/**
 * @brief Represents a list of YiniValue elements, created with `List(...)`.
 */
struct YiniList
{
  std::vector<YiniValue> elements;
  bool operator==(const YiniList& other) const { return elements == other.elements; }
};

/**
 * @brief Represents a set of YiniValue elements, created with `Set(...)`.
 * @note Uniqueness is enforced by the parser using `operator==`.
 */
struct YiniSet
{
  std::vector<YiniValue> elements;
  bool operator==(const YiniSet& other) const { return elements == other.elements; }
};

/**
 * @brief Represents a map of string keys to YiniValue elements, created with
 * `{...}`.
 */
struct YiniMap
{
  std::map<std::string, YiniValue> elements;
  bool operator==(const YiniMap& other) const { return elements == other.elements; }
};

/**
 * @brief Represents a single key-value pair, created with `{key: value}`.
 * @note This is a memory-efficient alternative to YiniMap for single pairs.
 */
struct YiniPair
{
  std::string key;
  YiniValue value;
  bool operator==(const YiniPair &other) const
  {
    return key == other.key && value == other.value;
  }
};

/**
 * @brief Represents a dynamic value that can be written back to the source file,
 * created with `Dyna(...)`.
 */
struct YiniDynaValue
{
  YiniValue value;
  bool operator==(const YiniDynaValue& other) const { return value == other.value; }
};

/**
 * @brief Represents a 2D or 3D coordinate, created with `Coord(...)`.
 */
struct YiniCoord
{
  double x, y, z;
  bool is_3d;
  bool operator==(const YiniCoord& other) const {
    return x == other.x && y == other.y && is_3d == other.is_3d && (!is_3d || z == other.z);
  }
};

/**
 * @brief Represents an RGB color, created with `#RRGGBB` or `Color(...)`.
 */
struct YiniColor
{
  unsigned char r, g, b;
  bool operator==(const YiniColor& other) const {
    return r == other.r && g == other.g && b == other.b;
  }
};

/**
 * @brief Represents a file path, created with `Path(...)`.
 */
struct YiniPath
{
  std::string pathValue;
  bool operator==(const YiniPath& other) const { return pathValue == other.pathValue; }
};

/**
 * @brief Represents a key-value pair within a section.
 */
struct YiniKeyValuePair
{
  std::string key;
  YiniValue value;
};

/**
 * @brief Represents a section in a YINI file, identified by `[...]`.
 */
struct YiniSection
{
  std::string name;                     ///< The name of the section.
  std::vector<std::string> inheritedSections; ///< List of sections this section inherits from.
  std::vector<YiniKeyValuePair> pairs;  ///< List of key-value pairs in this section.
  std::vector<YiniValue> registrationList; ///< List of values from quick registration (`+=`).
};

/**
 * @brief The root of the YINI object model, representing a full document.
 * @details This class manages all sections and macro definitions within a YINI
 * file. It provides methods for accessing, modifying, and processing the data,
 *          including resolving inheritance.
 */
class YiniDocument
{
public:
  // Rule of Five for thread-safe copying and moving
  YiniDocument() = default;
  YiniDocument(const YiniDocument &other);
  YiniDocument &operator=(const YiniDocument &other);
  YiniDocument(YiniDocument &&other) noexcept;
  YiniDocument &operator=(YiniDocument &&other) noexcept;

  /**
   * @brief Adds a section to the document.
   * @param section The section to add.
   */
  void addSection(const YiniSection &section)
  {
    std::lock_guard<std::mutex> lock(docMutex);
    sectionList.push_back(section);
  }

  /**
   * @brief Adds a section to the document by moving it.
   * @param section The section to move.
   */
  void addSection(YiniSection &&section)
  {
    std::lock_guard<std::mutex> lock(docMutex);
    sectionList.push_back(std::move(section));
  }

  /**
   * @brief Gets a copy of the list of sections in a thread-safe manner.
   * @return A vector of YiniSection.
   */
  std::vector<YiniSection> getSections() const
  {
    std::lock_guard<std::mutex> lock(docMutex);
    return sectionList;
  }

public:
  /**
   * @brief Adds a macro definition to the document's define map.
   * @param key The macro's key.
   * @param value The macro's value.
   */
  void addDefine(const std::string &key, const YiniValue &value)
  {
    std::lock_guard<std::mutex> lock(docMutex);
    defineMap[key] = value;
  }

  /**
   * @brief Retrieves a macro's value by its key.
   * @param key The key of the macro to retrieve.
   * @param[out] value The output value if the macro is found.
   * @return True if the macro was found, false otherwise.
   */
  bool getDefine(const std::string &key, YiniValue &value) const
  {
    std::lock_guard<std::mutex> lock(docMutex);
    auto it = defineMap.find(key);
    if (it != defineMap.end())
    {
      value = it->second;
      return true;
    }
    return false;
  }

  /**
   * @brief Gets a copy of the map of all defined macros.
   * @return A map of string to YiniValue.
   */
  std::map<std::string, YiniValue> getDefines() const
  {
    std::lock_guard<std::mutex> lock(docMutex);
    return defineMap;
  }

public:
  /**
   * @brief Resolves the inheritance graph for all sections in the document.
   * @details This method processes the `inheritedSections` list for each
   *          section, merging key-value pairs from parents into children.
   *          It also detects and throws an exception for circular dependencies.
   */
  void resolveInheritance();

private:
  /**
   * @brief (Private) Finds a section without locking the mutex.
   * @note For internal use by methods that already hold the lock.
   */
  YiniSection *findSectionInternal(const std::string &name)
  {
    auto it =
        std::find_if(sectionList.begin(), sectionList.end(),
                     [&](const YiniSection &s) { return s.name == name; });
    return it != sectionList.end() ? &(*it) : nullptr;
  }

  /**
   * @brief Recursively resolves inheritance for a single section.
   * @param section The section to resolve.
   * @param path The current inheritance path, used for cycle detection.
   * @param resolved A set of already resolved section names to avoid redundant
   * work.
   */
  void resolveSectionInheritance(YiniSection *section,
                                 std::vector<std::string> &path,
                                 std::set<std::string> &resolved);

public:
  /**
   * @brief Finds a section by its name (const version).
   * @param name The name of the section to find.
   * @return A constant pointer to the section if found, otherwise nullptr.
   */
  const YiniSection *findSection(const std::string &name) const
  {
    std::lock_guard<std::mutex> lock(docMutex);
    auto it =
        std::find_if(sectionList.begin(), sectionList.end(),
                     [&](const YiniSection &s) { return s.name == name; });

    if (it != sectionList.end())
    {
      return &(*it);
    }

    return nullptr;
  }

  /**
   * @brief Finds a section by name, creating it if it does not exist.
   * @param name The name of the section to get or create.
   * @return A pointer to the existing or newly created section.
   */
  YiniSection *getOrCreateSection(const std::string &name)
  {
    std::lock_guard<std::mutex> lock(docMutex);
    auto it =
        std::find_if(sectionList.begin(), sectionList.end(),
                     [&](const YiniSection &s) { return s.name == name; });

    if (it != sectionList.end())
    {
      return &(*it);
    }
    else
    {
      sectionList.push_back({name});
      return &sectionList.back();
    }
  }

  /**
   * @brief Merges another YiniDocument into this one.
   * @details Merges macros and section data from another document. Existing
   *          keys are overwritten, and new keys or sections are added.
   * @param other The document to merge from.
   */
  void merge(const YiniDocument &other)
  {
    std::lock_guard<std::mutex> lock(docMutex);
    // Note: 'other' document should be locked if accessed concurrently,
    // but this method only locks 'this'. Caller must ensure safety of 'other'.
    for (const auto &[key, value] : other.getDefines())
    {
      this->defineMap[key] = value;
    }

    for (const auto &other_section : other.getSections())
    {
      if (other_section.name == "#include" || other_section.name == "#define")
        continue;

      YiniSection *target_section = nullptr;
      auto it = std::find_if(this->sectionList.begin(), this->sectionList.end(),
                             [&](const YiniSection &s) { return s.name == other_section.name; });

      if (it != this->sectionList.end())
      {
        target_section = &(*it);
      }
      else
      {
        this->sectionList.push_back({other_section.name});
        target_section = &this->sectionList.back();
      }


      for (const auto &other_pair : other_section.pairs)
      {
        auto it_pair = std::find_if(
            target_section->pairs.begin(), target_section->pairs.end(),
            [&](const YiniKeyValuePair &p) { return p.key == other_pair.key; });

        if (it_pair != target_section->pairs.end())
        {
          it_pair->value = other_pair.value;
        }
        else
        {
          target_section->pairs.push_back(other_pair);
        }
      }

      target_section->registrationList.insert(
          target_section->registrationList.end(),
          other_section.registrationList.begin(),
          other_section.registrationList.end());
    }
  }

private:
  std::vector<YiniSection> sectionList;
  std::map<std::string, YiniValue> defineMap;
  mutable std::mutex docMutex;
};
} // namespace YINI

#endif // YINI_DATA_HPP