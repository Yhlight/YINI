#pragma once

#include "Interpreter/Interpreter.h"
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
    struct DirtyValue
    {
        YiniValue value;
        int line;
        int column;
    };

    class YiniManager
    {
    public:
        YiniManager();
        void load(const std::string& filepath);
        void save_changes();
        YiniValue get_value(const std::string& section, const std::string& key);
        void set_value(const std::string& section, const std::string& key, YiniValue value);

        Interpreter interpreter;

    private:
        std::vector<std::unique_ptr<Stmt>> load_file(const std::string& filepath, std::set<std::string>& loaded_files);
        void merge_asts(std::vector<std::unique_ptr<Stmt>>& base_ast, std::vector<std::unique_ptr<Stmt>>& new_ast);

        std::string m_filepath;
        std::map<std::string, std::map<std::string, DirtyValue>> m_dirty_values;
    };
}