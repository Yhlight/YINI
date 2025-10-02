#pragma once

#include "Interpreter/Interpreter.h"
#include "Parser/Ast.h"
#include "Core/DynaValue.h"
#include <string>
#include <vector>
#include <memory>
#include <set>
#include <map>

namespace YINI
{
    class YiniManager
    {
    public:
        YiniManager();
        void load(const std::string& filepath);
        void save_changes();
        std::any get_value(const std::string& section, const std::string& key);
        void set_value(const std::string& section, const std::string& key, std::any value);

        Interpreter interpreter;

    private:
        std::vector<std::unique_ptr<Stmt>> load_file(const std::string& filepath, std::set<std::string>& loaded_files);
        void merge_asts(std::vector<std::unique_ptr<Stmt>>& base_ast, std::vector<std::unique_ptr<Stmt>>& new_ast);

        std::string m_filepath;
        std::map<std::string, std::map<std::string, std::any>> m_dirty_values;
    };
}