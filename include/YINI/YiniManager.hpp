/**
 * @file YiniManager.hpp
 * @brief Defines the main high-level interface for managing YINI files.
 *
 * This file contains the YiniManager class, which orchestrates the loading,
 * parsing, caching, and persistence of YINI documents.
 */

#ifndef YINI_MANAGER_HPP
#define YINI_MANAGER_HPP

#include "YiniData.hpp"
#include <memory>
#include <string>

namespace YINI
{
/**
 * @class YiniManager
 * @brief Manages the entire lifecycle of a YINI file.
 * @details This class is the primary entry point for interacting with a YINI
 * file. It handles:
 * - Loading a `.yini` file or its cached `.ymeta` counterpart.
 * - Caching the parsed document to a `.ymeta` file for performance.
 * - Persisting changes to dynamic values (`Dyna()`) back to the `.yini` file
 * upon destruction.
 * - Providing an interface to modify values in the document.
 */
class YiniManager
{
public:
  /**
   * @brief Constructs a YiniManager and loads a document.
   * @details Upon construction, the manager attempts to load the specified
   * `.yini` file. It will prioritize loading from a `.ymeta` cache file if it
   * exists and is up-to-date.
   * @param yini_file_path The path to the `.yini` file to manage.
   */
  explicit YiniManager(const std::string &yini_file_path);

  /**
   * @brief Destructor for the YiniManager.
   * @details Automatically calls `write_back_dyna_values()` to persist any
   * changes to dynamic values before the object is destroyed.
   */
  ~YiniManager();

  /**
   * @brief Gets read-only access to the underlying YiniDocument.
   * @return A constant reference to the YiniDocument.
   */
  const YiniDocument &get_document() const;

  /**
   * @brief Checks if the document was successfully loaded.
   * @return True if a document is loaded, false otherwise.
   */
  bool is_loaded() const;

  /**
   * @brief Sets a string value for a key in a given section.
   * @details If the key holds a `Dyna()` value, its inner value is updated.
   *          The document is saved to its `.ymeta` cache after the change.
   * @param section The name of the section.
   * @param key The key of the value to set.
   * @param value The string value to set.
   */
  void set_string_value(const std::string &section, const std::string &key,
                      const std::string &value);

  /**
   * @brief Sets an integer value for a key in a given section.
   * @see set_string_value for more details on behavior.
   * @param section The name of the section.
   * @param key The key of the value to set.
   * @param value The integer value to set.
   */
  void set_int_value(const std::string &section, const std::string &key,
                   int value);

  /**
   * @brief Sets a double value for a key in a given section.
   * @see set_string_value for more details on behavior.
   * @param section The name of the section.
   * @param key The key of the value to set.
   * @param value The double value to set.
   */
  void set_double_value(const std::string &section, const std::string &key,
                      double value);

  /**
   * @brief Sets a boolean value for a key in a given section.
   * @see set_string_value for more details on behavior.
   * @param section The name of the section.
   * @param key The key of the value to set.
   * @param value The boolean value to set.
   */
  void set_bool_value(const std::string &section, const std::string &key,
                    bool value);

private:
  /**
   * @brief Loads the document from cache or source file.
   * @return True on success, false on failure.
   */
  bool load_document();

  /**
   * @brief Saves the current document state to the `.ymeta` file.
   * @return True on success, false on failure.
   */
  bool save_document();

  /**
   * @brief Rewrites the source `.yini` file to update dynamic values.
   */
  void write_back_dyna_values();

  std::string yini_file_path;  ///< Path to the source `.yini` file.
  std::string ymeta_file_path; ///< Path to the cached `.ymeta` file.
  YiniDocument document;     ///< The in-memory representation of the document.
  bool m_is_loaded;            ///< Flag indicating if the document was loaded successfully.
};
} // namespace YINI

#endif // YINI_MANAGER_HPP