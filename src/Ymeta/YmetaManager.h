#pragma once

#include "YiniTypes.h"
#include <map>
#include <string>
#include <vector>

namespace YINI
{

/**
 * @brief Manages the loading, saving, and accessing of dynamic values in .ymeta files.
 * @details .ymeta files are JSON-based companions to .yini files that store the state of
 *          dynamic (`Dyna()`) values. This manager handles the serialization/deserialization
 *          of these values and provides a backup mechanism.
 */
class YmetaManager
{
  public:
    /// @brief Constructs a new YmetaManager object.
    YmetaManager();

    /**
     * @brief Loads dynamic values from a .ymeta file corresponding to a given .yini file.
     * @param yini_filepath The path to the original .yini file. The manager will look for a
     *                      corresponding .ymeta file (e.g., "config.yini" -> "config.ymeta").
     */
    void load(const std::string &yini_filepath);

    /**
     * @brief Saves the current state of dynamic values to a .ymeta file.
     * @param yini_filepath The path to the original .yini file, used to determine the .ymeta path.
     */
    void save(const std::string &yini_filepath);

    /**
     * @brief Checks if a dynamic value for a given key exists.
     * @param key The key to check (e.g., "Section.Key").
     * @return True if the value exists, false otherwise.
     */
    bool has_value(const std::string &key);

    /**
     * @brief Gets the dynamic value for a given key.
     * @param key The key of the value to retrieve.
     * @return The YiniVariant containing the value.
     */
    YiniVariant get_value(const std::string &key);

    /**
     * @brief Sets or updates a dynamic value.
     * @details Also handles the backup mechanism, storing the previous value if one existed.
     * @param key The key of the value to set.
     * @param value The new YiniVariant value to store.
     */
    void set_value(const std::string &key, YiniVariant value);

  private:
    std::string get_ymeta_path(const std::string &yini_filepath);

    std::map<std::string, YiniVariant> m_dynamic_values;
    std::map<std::string, std::vector<YiniVariant>> m_backup_values;
};

} // namespace YINI
