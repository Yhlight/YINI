#pragma once

#include "Lexer/Token.h"
#include <string>
#include <map>
#include <any>

namespace YINI
{
    class Environment
    {
    public:
        void define(const std::string& name, std::any value);
        std::any get(const Token& name);
        const std::map<std::string, std::any>& get_all() const;

    private:
        std::map<std::string, std::any> m_values;
    };
}