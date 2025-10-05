#pragma once

#include "Lexer/Token.h"
#include "Core/YiniValue.h"
#include <map>
#include <string>

namespace YINI
{
    class Environment
    {
    public:
        void define(const std::string& name, YiniValue value);
        YiniValue get(const Token& name) const;
        void clear();
        std::vector<std::string> get_all_keys() const;

    private:
        std::map<std::string, YiniValue> m_values;
    };
}