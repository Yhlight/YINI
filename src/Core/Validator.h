#pragma once

#include <string>
#include <vector>

#include "Interpreter/Interpreter.h"
#include "Parser/Ast.h"

namespace YINI {
/**
 * @struct ValidationError
 * @brief Represents a single validation error found by the Validator.
 */
struct ValidationError {
    std::string message;
    // We could add line/column info here later if needed
};

/**
 * @class Validator
 * @brief Validates a YINI configuration against a defined schema.
 */
class Validator {
public:
    /**
     * @brief Validates the loaded configuration against the provided schema.
     * @param schema The Schema AST node containing the validation rules.
     * @param interpreter The interpreter instance holding the resolved configuration data.
     * @return A vector of validation errors. An empty vector means validation passed.
     */
    std::vector<ValidationError> validate(const Schema& schema, const Interpreter& interpreter);

private:
    // Helper methods for validation will be added here
    void validate_type(const std::string& type_str, const YiniValue& value, const std::string& full_key,
                       std::vector<ValidationError>& errors);
};
}  // namespace YINI