/**
 * @file YiniManager.h
 * @brief Defines the main YiniManager class for interacting with YINI files.
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
     * @brief Represents a value that has been modified but not yet saved to the file.
     */
    struct DirtyValue
    {
        YiniValue value; //!< The new value.
        int line;        //!< The original line number of the value (0 if it's a new value).
        int column;      //!< The original column number of the value.
    };

    /**
     * @class YiniManager
     * @brief The main entry point for the YINI library.
     *
     * This class manages the loading, parsing, querying, and saving of YINI files.
     * It provides a high-level API for interacting with configuration data.
     */
    class YiniManager
    {
    public:
        /**
         * @brief Default constructor.
         */
        YiniManager();

        /**
         * @brief Loads and parses a YINI file.
         * @param filepath The path to the .yini file to load.
         * @throws std::runtime_error if the file cannot be opened.
         * @throws YiniException on parsing or interpretation errors.
         */
        void load(const std::string& filepath);

        /**
         * @brief Saves any modified dynamic values back to the original file.
         *
         * This method performs a non-destructive write-back, preserving comments
         * and formatting where possible. New values are appended to their respective
         * sections.
         * @throws std::runtime_error if the file cannot be written.
         */
        void save_changes();

        /**
         * @brief Retrieves a value from a specific section and key.
         * @param section The name of the section.
         * @param key The name of the key.
         * @return The requested YiniValue. If the value is dynamic, this returns the underlying value.
         * @throws std::runtime_error if the section or key is not found.
         */
        YiniValue get_value(const std::string& section, const std::string& key);

        /**
         * @brief Sets the value for a given section and key.
         *
         * This operation is only valid for values that were originally declared as
         * dynamic with `Dyna()`. If the key does not exist but the section does,
         * a new dynamic value will be created.
         * @param section The name of the section.
         * @param key The name of the key.
         * @param value The new value to set.
         * @throws std::runtime_error if the section does not exist or if the key
         *         exists but is not a dynamic value.
         */
        void set_value(const std::string& section, const std::string& key, YiniValue value);

        /**
         * @brief The interpreter instance that holds the resolved state of the YINI file.
         *
         * This is public to allow for advanced inspection of the parsed data, but
         * typical use cases should rely on get_value() and set_value().
         */
        const Interpreter& get_interpreter() const;

        /**
         * @brief Gets the parsed schema, if one was present in the loaded files.
         * @return A const pointer to the Schema AST node, or nullptr if no schema was found.
         */
        const Schema* get_schema() const;

    private:
        std::vector<std::unique_ptr<Stmt>> load_file(const std::string& filepath, std::set<std::string>& loaded_files);
        void merge_asts(std::vector<std::unique_ptr<Stmt>>& base_ast, std::vector<std::unique_ptr<Stmt>>& new_ast);

        std::string m_filepath;
        Interpreter m_interpreter;
        std::map<std::string, std::map<std::string, DirtyValue>> m_dirty_values;
        std::unique_ptr<Schema> m_schema;
    };
}