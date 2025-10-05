#pragma once

#include <map>
#include <optional>
#include <string>
#include <string_view>

#include "Core/YiniValue.h"
#include "Lexer/Token.h"

namespace YINI {
/**
 * @struct MacroDefinition
 * @brief Holds the value and definition location of a macro.
 */
struct MacroDefinition {
    YiniValue value;
    Token definition_token;
};

class Environment {
public:
    void define(const Token& name_token, YiniValue value);
    YiniValue get(const Token& name) const;
    std::optional<Token> get_definition_token(std::string_view name) const;
    void clear();
    std::vector<std::string> get_all_keys() const;

private:
    std::map<std::string, MacroDefinition, std::less<>> m_values;
};
}  // namespace YINI