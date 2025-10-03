/**
 * @file YiniManager.h
 * @brief Defines the main YiniManager class for interacting with YINI files.
 * @ingroup Core
 */
#pragma once

#include "Interpreter/Interpreter.h"
#include "Parser/Ast.h"
#include "Core/DynaValue.h"
#include "Core/YiniValue.h"
#include <string>
#include <vector>
#include <memory>
#include <set>
#include <map>

namespace YINI
{
    /**
     * @struct DirtyValue
     * @brief Represents a dynamic value that has been modified at runtime.
     * @ingroup Core
     */
    struct DirtyValue
    {
        YiniValue value; ///< The new, updated value.
        int line;        ///< The original line number of the value. A value of 0 indicates a newly added value.
        int column;      ///< The original column number of the value.
    };

    /**
     * @class YiniManager
     * @brief The primary high-level API for interacting with YINI files.
     * @ingroup Core
     *
     * @details This class acts as a facade, managing the loading, parsing, querying,
     * and saving of YINI configuration data. It handles file includes, resolves
     * section inheritance, and tracks changes to dynamic values.
     */
    class YiniManager
    {
    public:
        /// @name Constructors
        /// @{

        /**
         * @brief Default constructor.
         */
        YiniManager();

        /// @}

        /// @name File I/O
        /// @{

        /**
         * @brief Loads and parses a YINI file and all its includes.
         *
         * @details This method reads the specified file, parses its content, handles
         * any `[#include]` directives recursively, and populates the interpreter
         * with the final, resolved configuration state.
         *
         * @param filepath The path to the .yini file to load.
         * @throws std::runtime_error if the root file cannot be opened.
         * @throws YiniException on parsing, interpretation, or semantic errors.
         */
        void load(const std::string& filepath);

        /**
         * @brief Saves any modified dynamic values back to the source file.
         *
         * @details This method performs a non-destructive write-back operation. It
         * reads the original file line by line and replaces or appends values that
         * have been modified via `set_value()`. Comments and file formatting are
         * preserved as much as possible.
         *
         * @throws std::runtime_error if the source file cannot be read or written to.
         */
        void save_changes();

        /// @}

        /// @name Value Access
        /// @{

        /**
         * @brief Retrieves a value from the configuration.
         *
         * @details This method searches for the specified key within the given section
         * and returns its value. If the value is dynamic (i.e., wrapped in `Dyna()`),
         * this method returns the underlying, concrete value.
         *
         * @param section The name of the section.
         * @param key The name of the key.
         * @return A copy of the requested YiniValue.
         * @throws std::runtime_error if the section or key is not found.
         * @see set_value()
         */
        YiniValue get_value(const std::string& section, const std::string& key);

        /**
         * @brief Modifies the value of a dynamic key or adds a new dynamic key.
         *
         * @details This operation is only valid for values that were originally
         * declared as dynamic with `Dyna()`. If the key does not exist but the
         * section does, a new dynamic value will be created and marked as "dirty"
         * for `save_changes()`.
         *
         * @param section The name of the section.
         * @param key The name of the key.
         * @param value The new value to set.
         * @throws std::runtime_error if the section does not exist or if the key
         *         exists but is not a dynamic value.
         * @see get_value(), save_changes()
         */
        void set_value(const std::string& section, const std::string& key, YiniValue value);

        /// @}

        /**
         * @brief The interpreter instance that holds the resolved state of the YINI file.
         *
         * @warning This is exposed for advanced use cases, such as iterating over
         * all sections and keys. For simple value access, it is strongly
         * recommended to use `get_value()` and `set_value()`.
         */
        Interpreter interpreter;

    private:
        /// @private
        std::vector<std::unique_ptr<Stmt>> load_file(const std::string& filepath, std::set<std::string>& loaded_files);
        /// @private
        void merge_asts(std::vector<std::unique_ptr<Stmt>>& base_ast, std::vector<std::unique_ptr<Stmt>>& new_ast);

        /// @brief The path to the root YINI file that was loaded.
        std::string m_filepath;
        /// @brief A map to track values that have been modified via `set_value`.
        std::map<std::string, std::map<std::string, DirtyValue>> m_dirty_values;
    };
}