#pragma once

#include "../Parser/AST.h"
#include <memory>
#include <string>
#include <variant>

namespace YINI
{
    // A variant-like object to represent the result of an evaluation
    struct YiniObject
    {
        enum class Type
        {
            INTEGER,
            FLOAT,
            BOOLEAN,
            STRING,
            ERROR
        };

        Type type;
        std::variant<int64_t, double, bool, std::string> value;
    };

    class Evaluator
    {
    public:
        std::unique_ptr<YiniObject> evaluate(AST::Node* node);

    private:
        std::unique_ptr<YiniObject> evaluateInfixExpression(AST::InfixExpression* node);
        // ... other evaluation methods for different node types
    };
}
