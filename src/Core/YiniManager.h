#pragma once

#include "Interpreter/Interpreter.h"
#include "Parser/Ast.h"
#include <string>
#include <vector>
#include <memory>
#include <set>

namespace YINI
{
    class YiniManager
    {
    public:
        YiniManager();
        void load(const std::string& filepath);

        Interpreter interpreter;

    private:
        std::vector<std::unique_ptr<Stmt>> load_file(const std::string& filepath, std::set<std::string>& loaded_files);
        void merge_asts(std::vector<std::unique_ptr<Stmt>>& base_ast, std::vector<std::unique_ptr<Stmt>>& new_ast);
    };
}