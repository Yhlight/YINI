#pragma once

#include "Ast.h"
#include "../Lexer/Token.h"
#include <optional>

// Forward declaration to avoid circular dependency
class Parser;

class ValueParsingStrategy
{
public:
    virtual ~ValueParsingStrategy() = default;
    // The `try_parse` method will check if the strategy can handle the current token(s) and parse if it can.
    // It returns an empty optional if it cannot handle the current token.
    virtual std::optional<YiniValue> try_parse(Parser& parser) = 0;
};

// --- Concrete Strategy Declarations ---

// Handles "string literals"
class StringStrategy : public ValueParsingStrategy {
public:
    std::optional<YiniValue> try_parse(Parser& parser) override;
};

// Handles integer, float, and arithmetic expressions
class NumericExpressionStrategy : public ValueParsingStrategy {
public:
    std::optional<YiniValue> try_parse(Parser& parser) override;
};

// Handles true/false
class BoolStrategy : public ValueParsingStrategy {
public:
    std::optional<YiniValue> try_parse(Parser& parser) override;
};

// Handles [...]
class ArrayStrategy : public ValueParsingStrategy {
public:
    std::optional<YiniValue> try_parse(Parser& parser) override;
};

// Handles path(...)
class PathStrategy : public ValueParsingStrategy {
public:
    std::optional<YiniValue> try_parse(Parser& parser) override;
};

// Handles coord(...)
class CoordStrategy : public ValueParsingStrategy {
public:
    std::optional<YiniValue> try_parse(Parser& parser) override;
};

// Handles color(...) and #RRGGBB
class ColorStrategy : public ValueParsingStrategy {
public:
    std::optional<YiniValue> try_parse(Parser& parser) override;
};

// Handles {...}
class ObjectStrategy : public ValueParsingStrategy {
public:
    std::optional<YiniValue> try_parse(Parser& parser) override;
};

// Handles @macro
class MacroRefStrategy : public ValueParsingStrategy {
public:
    std::optional<YiniValue> try_parse(Parser& parser) override;
};
