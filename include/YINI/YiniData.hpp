#ifndef YINI_DATA_HPP
#define YINI_DATA_HPP

#include <algorithm>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <variant>
#include <vector>

namespace YINI
{
// Forward declarations
struct YiniValue;
struct YiniArray;
struct YiniList;
struct YiniSet;
struct YiniTuple;
struct YiniMap;
struct YiniDynaValue;
struct YiniCoord;
struct YiniColor;
struct YiniPath;

/**
 * @brief A variant type that can hold any of the possible YINI data types.
 * @note Recursive types are held by unique_ptr to break circular dependencies.
 */
using YiniVariant =
    std::variant<std::string, int, double, bool, std::unique_ptr<YiniArray>,
                 std::unique_ptr<YiniList>, std::unique_ptr<YiniSet>,
                 std::unique_ptr<YiniTuple>, std::unique_ptr<YiniMap>,
                 std::unique_ptr<YiniDynaValue>, std::unique_ptr<YiniCoord>,
                 std::unique_ptr<YiniColor>, std::unique_ptr<YiniPath>>;

/**
 * @brief Represents a single value in a YINI document.
 *
 * This struct is a wrapper around the YiniVariant, which allows for recursive
 * data structures like arrays of arrays. It also provides comparison operators
 * to be used in containers like std::set.
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

  /**
   * @brief Compares this YiniValue with another for ordering.
   * @param other The YiniValue to compare against.
   * @return true if this value is less than the other, false otherwise.
   */
  bool operator<(const YiniValue &other) const;
};

/** @brief Represents a YINI array `[...]` or `Array(...)`. */
struct YiniArray
{
  std::vector<YiniValue> elements;
  bool operator<(const YiniArray &other) const;
};

/** @brief Represents a YINI list `List(...)`. */
struct YiniList
{
  std::vector<YiniValue> elements;
  bool operator<(const YiniList &other) const;
};

/** @brief Represents a YINI set `Set(...)`, which enforces uniqueness. */
struct YiniSet
{
  std::set<YiniValue> elements;
  bool operator<(const YiniSet &other) const;
};

/** @brief Represents an optimized, single key-value pair `{key: value}`. */
struct YiniTuple
{
  std::string key;
  YiniValue value;
  bool operator<(const YiniTuple &other) const;
};

/** @brief Represents a YINI map `{...}`. */
struct YiniMap
{
  std::map<std::string, YiniValue> elements;
  bool operator<(const YiniMap &other) const;
};

/** @brief Internal wrapper for values marked with `Dyna()`. */
struct YiniDynaValue
{
  YiniValue value;
  bool operator<(const YiniDynaValue &other) const;
};

/** @brief Represents a 2D or 3D coordinate `Coord(...)`. */
struct YiniCoord
{
  double x, y, z;
  bool is_3d;
  bool operator<(const YiniCoord &other) const;
};

/** @brief Represents a color, parsed from `#RRGGBB` or `Color(...)`. */
struct YiniColor
{
  unsigned char r, g, b;
  bool operator<(const YiniColor &other) const;
};

/** @brief Represents a file path `Path(...)`. */
struct YiniPath
{
  std::string path_value;
  bool operator<(const YiniPath &other) const;
};

/** @brief Represents a location in a source file. */
struct Location {
    int line = 0;
    int column = 0;
};

/** @brief Represents a macro definition, including its value and location. */
struct YiniDefine {
    YiniValue value;
    Location location;
};

/** @brief A key-value pair within a YINI section. */
struct YiniKeyValuePair
{
  std::string key;
  YiniValue value;
  bool is_dynamic = false; ///< True if the value was declared with `Dyna()`.
};

/** @brief Represents a single section `[...]` in a YINI document. */
struct YiniSection
{
  std::string name;
  std::vector<std::string> inheritedSections;
  std::vector<YiniKeyValuePair> pairs;
  std::vector<YiniValue> registrationList;
};

/**
 * @brief Represents a full YINI document, containing all sections and macros.
 *
 * This class is the root of the parsed YINI data structure. It provides
 * methods for accessing and manipulating the document's content.
 */
class YiniDocument
{
public:
  /** @brief Adds a new section to the document. */
  void addSection(const YiniSection &section) { sections.push_back(section); }

  /** @brief Adds a new section to the document. */
  void addSection(YiniSection &&section)
  {
    sections.push_back(std::move(section));
  }

  /** @brief Gets a mutable reference to the vector of sections. */
  std::vector<YiniSection> &getSections() { return sections; }

  /** @brief Gets a constant reference to the vector of sections. */
  const std::vector<YiniSection> &getSections() const { return sections; }

public:
  /**
   * @brief Finds a section by its name.
   * @param name The name of the section to find.
   * @return A pointer to the section if found, otherwise nullptr.
   */
  YiniSection *findSection(const std::string &name)
  {
    auto it =
        std::find_if(sections.begin(), sections.end(),
                     [&](const YiniSection &s) { return s.name == name; });
    return (it != sections.end()) ? &(*it) : nullptr;
  }

  /** @brief Adds a new macro definition to the document's global scope. */
  void addDefine(const std::string &key, const YiniValue &value, int line, int column)
  {
    defines[key] = {value, {line, column}};
  }

  /**
   * @brief Retrieves a macro definition by its key.
   * @param key The key of the macro.
   * @param[out] define The YiniDefine struct to populate.
   * @return true if the macro was found, false otherwise.
   */
  bool getDefine(const std::string &key, YiniDefine &define) const
  {
    auto it = defines.find(key);
    if (it != defines.end())
    {
      define = it->second;
      return true;
    }
    return false;
  }

  /** @brief Gets a constant reference to the map of all defined macros. */
  const std::map<std::string, YiniDefine> &getDefines() const { return defines; }

public:
  /** @brief Resolves all section inheritance for the document. */
  void resolveInheritance();

private:
  void resolveSectionInheritance(YiniSection *section,
                                 std::vector<std::string> &path,
                                 std::set<std::string> &resolved);

public:
  /**
   * @brief Finds a section by its name (const version).
   * @param name The name of the section to find.
   * @return A pointer to the section if found, otherwise nullptr.
   */
  const YiniSection *findSection(const std::string &name) const
  {
    auto it =
        std::find_if(sections.begin(), sections.end(),
                     [&](const YiniSection &s) { return s.name == name; });
    return (it != sections.end()) ? &(*it) : nullptr;
  }

  /**
   * @brief Gets a section by name, creating it if it doesn't exist.
   * @param name The name of the section.
   * @return A pointer to the existing or newly created section.
   */
  YiniSection *getOrCreateSection(const std::string &name)
  {
    auto it =
        std::find_if(sections.begin(), sections.end(),
                     [&](const YiniSection &s) { return s.name == name; });
    if (it != sections.end()) { return &(*it); }
    else
    {
      sections.push_back({name});
      return &sections.back();
    }
  }

  /** @brief Merges another document's contents into this one. */
  void merge(const YiniDocument &other)
  {
    for (const auto &[key, define] : other.getDefines())
    {
      this->addDefine(key, define.value, define.location.line, define.location.column);
    }

    for (const auto &other_section : other.getSections())
    {
      if (other_section.name == "#include") continue;
      YiniSection *target_section = getOrCreateSection(other_section.name);
      if (other_section.name == "#define") continue;

      for (const auto &other_pair : other_section.pairs)
      {
        auto it = std::find_if(
            target_section->pairs.begin(), target_section->pairs.end(),
            [&](const YiniKeyValuePair &p) { return p.key == other_pair.key; });

        if (it != target_section->pairs.end()) { it->value = other_pair.value; }
        else { target_section->pairs.push_back(other_pair); }
      }

      target_section->registrationList.insert(
          target_section->registrationList.end(),
          other_section.registrationList.begin(),
          other_section.registrationList.end());
    }
  }

private:
  std::vector<YiniSection> sections;
  std::map<std::string, YiniDefine> defines;
};
} // namespace YINI

#endif // YINI_DATA_HPP