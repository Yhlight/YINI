#pragma once

#include <string>
#include <vector>
#include <memory>

namespace YINI
{
    // Base class for all nodes in the AST
    struct Node {
        virtual ~Node() = default;
        virtual std::string String() const = 0;
    };

    // Base class for all statements
    struct Statement : public Node {
    };

    // Represents a key-value pair, e.g., key = "value"
    struct KeyValuePair : public Statement {
        std::string key;
        // In the future, this will be a more complex Expression node
        std::string value;

        std::string String() const override {
            return key + " = " + value;
        }
    };

    // Represents a section, e.g., [Config]
    struct Section : public Statement {
        std::string name;
        std::vector<std::shared_ptr<KeyValuePair>> pairs;

        std::string String() const override {
            return "[" + name + "]";
        }
    };

    // The root node of the entire YINI file
    struct Program : public Node {
        std::vector<std::shared_ptr<Statement>> statements;

        std::string String() const override {
            std::string out;
            for (const auto& s : statements) {
                out += s->String() + "\n";
            }
            return out;
        }
    };
}