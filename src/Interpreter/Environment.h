#pragma once

#include "Lexer/Token.h"
#include "Core/YiniValue.h"
#include <map>
#include <string>
#include <optional>

namespace YINI
{
    /**
     * @struct MacroDefinition
     * @brief Holds the value and definition location of a macro.
     */
    struct MacroDefinition
    {
        YiniValue value;
        Token definition_token;
    };

    class Environment
    {
    public:
        void define(const Token& name_token, YiniValue value);
        YiniValue get(const Token& name) const;
        std::optional<Token> get_definition_token(const std::string& name) const;
        void clear();
        std::vector<std::string> get_all_keys() const;

    private:
        std::map<std::string, MacroDefinition> m_values;
    };
}