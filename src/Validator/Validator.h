#pragma once

#include "Parser/AST.h"
#include "YiniTypes.h"
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace YINI
{

/**
 * @brief Validates a resolved YINI configuration against schema definitions.
 * @details Takes the resolved configuration map from the Resolver and the original AST
 *          (which contains the schema definitions), and enforces the rules defined in
 *          `[#schema]` blocks. This can include applying default values for missing keys,
 *          checking types, and validating ranges.
 */
class Validator
{
  public:
    /**
     * @brief Constructs a new Validator object.
     * @param resolved_config A reference to the resolved configuration map from the Resolver.
     *                        This map may be modified by the validator (e.g., to add default values).
     * @param statements A reference to the vector of root AST statements, used to find schema definitions.
     */
    Validator(std::map<std::string, YiniVariant> &resolved_config,
              const std::vector<std::unique_ptr<AST::Stmt>> &statements);

    /**
     * @brief Runs the entire validation process.
     * @details Iterates through all schema definitions found in the AST and validates the
     *          corresponding sections and keys in the resolved configuration.
     * @throws std::runtime_error if a validation rule is violated (e.g., a required key is
     *         missing and no default is provided, or a type mismatch occurs).
     */
    void validate();

  private:
    void validate_section(const std::string &section_name, const AST::SchemaSectionStmt *schema_section);

    std::map<std::string, YiniVariant> &m_resolved_config;
    const std::vector<std::unique_ptr<AST::Stmt>> &m_statements;
};

} // namespace YINI
