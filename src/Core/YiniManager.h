#pragma once

#include "Interpreter/Interpreter.h"
#include "Parser/Ast.h"
#include "Core/DynaValue.h"
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <set>
#include <map>

namespace YINI
{
    struct DirtyValue
    {
        std::any value;
        int line;
        int column;
    };

    class YiniManager
    {
    public:
        YiniManager();
        void load(const std::string& filepath);
        void load_from_string(const std::string& content, const std::string& filepath = "");
        void save_changes();
        std::any get_value(const std::string& section, const std::string& key);
        void set_value(const std::string& section, const std::string& key, std::any value);

        Interpreter interpreter;

    private:
        std::vector<std::unique_ptr<Stmt>> load_file(const std::string& filepath, std::set<std::string>& loaded_files, const std::optional<std::string>& content = std::nullopt);
        void merge_asts(std::vector<std::unique_ptr<Stmt>>& base_ast, std::vector<std::unique_ptr<Stmt>>& new_ast);
        void update_line(std::string& line, const DirtyValue& dirty_value);

        std::string m_filepath;
        std::vector<std::string> m_lines;
        std::map<std::string, std::map<std::string, DirtyValue>> m_dirty_values;
    };
}