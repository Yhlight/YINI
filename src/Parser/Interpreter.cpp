#include "Interpreter.h"
#include <cstdlib>
#include <iostream>
#include <cmath> // For fmod

namespace yini
{

// Main entry point for the interpreter
bool Interpreter::interpret(RootNode& root)
{
    try
    {
        // First pass: collect all top-level definitions
        for (const auto& child : root.children)
        {
            if (auto define_node = std::dynamic_pointer_cast<DefineNode>(child))
            {
                visit(*define_node);
            }
            else if (auto include_node = std::dynamic_pointer_cast<IncludeNode>(child))
            {
                visit(*include_node);
            }
            else if (auto section_node = std::dynamic_pointer_cast<SectionNode>(child))
            {
                // Just add the section for now, will populate later
                sections[section_node->name] = Section(section_node->name);
                sections[section_node->name].inherited_sections = section_node->inherited_sections;
            }
        }

        // Second pass: evaluate all key-value pairs within sections
        for (const auto& child : root.children)
        {
            if (auto section_node = std::dynamic_pointer_cast<SectionNode>(child))
            {
                visit(*section_node);
            }
        }

        // Final pass: resolve inheritance
        for (auto& pair : sections)
        {
            Section& section = pair.second;
            if (section.inherited_sections.empty()) continue;

            std::map<std::string, std::shared_ptr<Value>> merged_entries;
            // Apply inherited sections first (in order)
            for (const auto& inherited_name : section.inherited_sections)
            {
                if (sections.count(inherited_name))
                {
                    for (const auto& entry : sections.at(inherited_name).entries)
                    {
                        merged_entries[entry.first] = entry.second;
                    }
                }
            }
            // Apply the section's own entries, overwriting inherited ones
            for (const auto& entry : section.entries)
            {
                merged_entries[entry.first] = entry.second;
            }
            section.entries = merged_entries;
        }

        // Final pass: resolve all references recursively
        for (auto& section_pair : sections)
        {
            for (auto& entry_pair : section_pair.second.entries)
            {
                std::set<std::string> visiting;
                std::string ref_path = section_pair.first + "." + entry_pair.first;
                visiting.insert(ref_path);
                entry_pair.second = resolveValue(entry_pair.second, visiting);
            }
        }
    }
    catch (const InterpreterError& e)
    {
        last_error = e.what();
        return false;
    }
    return true;
}

std::shared_ptr<Value> Interpreter::evaluate(ASTNode& node)
{
    node.accept(*this);
    return last_value;
}

void Interpreter::visit(RootNode& node)
{
    (void)node; // The main interpret loop handles the logic
}

void Interpreter::visit(SectionNode& node)
{
    current_section = node.name;
    for (const auto& kvp_node : node.children)
    {
        visit(*kvp_node);
    }
    current_section = "";
}

void Interpreter::visit(DefineNode& node)
{
    current_section = "[#define]";
    for (const auto& kvp_node : node.definitions)
    {
        defines[kvp_node->key] = evaluate(*kvp_node->value);
    }
    current_section = "";
}

void Interpreter::visit(IncludeNode& node)
{
    includes = node.files;
}

void Interpreter::visit(SchemaNode& node)
{
    (void)node; // Schema handling is not part of this refactoring
}

void Interpreter::visit(KeyValuePairNode& node)
{
    auto value = evaluate(*node.value);
    if (!current_section.empty() && sections.count(current_section))
    {
        sections.at(current_section).entries[node.key] = value;
    }
}

void Interpreter::visit(LiteralNode& node)
{
    last_value = node.value;
}

void Interpreter::visit(ArrayNode& node)
{
    Value::ArrayType elements;
    for (const auto& elem_node : node.elements)
    {
        elements.push_back(evaluate(*elem_node));
    }
    last_value = std::make_shared<Value>(elements);
}

void Interpreter::visit(MapNode& node)
{
    Value::MapType map;
    for (const auto& pair_node : node.pairs)
    {
        map[pair_node->key] = evaluate(*pair_node->value);
    }
    last_value = std::make_shared<Value>(map);
}

void Interpreter::visit(UnaryOpNode& node)
{
    auto right = evaluate(*node.right);
    if (node.op.type == TokenType::MINUS)
    {
        if (right->isInteger()) last_value = std::make_shared<Value>(-(right->asInteger()));
        else if (right->isFloat()) last_value = std::make_shared<Value>(-(right->asFloat()));
        else throw InterpreterError("Unary minus can only be applied to numbers.");
    }
}

void Interpreter::visit(BinaryOpNode& node)
{
    auto left = evaluate(*node.left);
    auto right = evaluate(*node.right);

    if (left->isNumeric() && right->isNumeric())
    {
        double l = left->asFloat();
        double r = right->asFloat();
        bool is_float = left->isFloat() || right->isFloat();

        double result;
        switch (node.op.type)
        {
            case TokenType::PLUS: result = l + r; break;
            case TokenType::MINUS: result = l - r; break;
            case TokenType::MULTIPLY: result = l * r; break;
            case TokenType::DIVIDE: result = l / r; break;
            case TokenType::MODULO: result = fmod(l, r); break;
            default: throw InterpreterError("Unknown binary operator.");
        }

        if (is_float) last_value = std::make_shared<Value>(result);
        else last_value = std::make_shared<Value>(static_cast<int64_t>(result));
    }
    else
    {
        throw InterpreterError("Binary operations can only be applied to numbers.");
    }
}

void Interpreter::visit(ReferenceNode& node)
{
    // Don't resolve here. Just create the reference value.
    // The final pass in interpret() will resolve it.
    last_value = Value::makeReference(node.name);
}

void Interpreter::visit(EnvVarNode& node)
{
    const char* env_val = std::getenv(node.name.c_str());
    if (env_val)
    {
        last_value = std::make_shared<Value>(std::string(env_val));
    }
    else
    {
        last_value = std::make_shared<Value>(std::string("")); // Return empty string if not found
    }
}

void Interpreter::visit(DynamicNode& node)
{
    auto inner_value = evaluate(*node.value);
    last_value = Value::makeDynamic(inner_value);
}

void Interpreter::visit(FunctionCallNode& node)
{
    std::vector<std::shared_ptr<Value>> args;
    for (const auto& arg_node : node.arguments)
    {
        args.push_back(evaluate(*arg_node));
    }

    if (node.callee_name == "Color" || node.callee_name == "color")
    {
        if (args.size() != 3 || !args[0]->isInteger() || !args[1]->isInteger() || !args[2]->isInteger())
            throw InterpreterError("Color() requires 3 integer arguments.");
        last_value = std::make_shared<Value>(Color(args[0]->asInteger(), args[1]->asInteger(), args[2]->asInteger()));
    }
    else if (node.callee_name == "Coord" || node.callee_name == "coord")
    {
        if ((args.size() != 2 && args.size() != 3) || !args[0]->isNumeric() || !args[1]->isNumeric() || (args.size() == 3 && !args[2]->isNumeric()))
            throw InterpreterError("Coord() requires 2 or 3 numeric arguments.");
        std::optional<double> z = (args.size() == 3) ? std::optional(args[2]->asFloat()) : std::nullopt;
        last_value = std::make_shared<Value>(Coord(args[0]->asFloat(), args[1]->asFloat(), z));
    }
    else if (node.callee_name == "Path" || node.callee_name == "path")
    {
        if (args.size() != 1 || !args[0]->isString())
            throw InterpreterError("Path() requires 1 string argument.");
        last_value = Value::makePath(args[0]->asString());
    }
    else if (node.callee_name == "List" || node.callee_name == "list")
    {
        last_value = Value::makeList(args);
    }
    else
    {
        throw InterpreterError("Unknown function call: " + node.callee_name);
    }
}

std::shared_ptr<Value> Interpreter::resolveReference(const std::string& name, std::set<std::string>& visiting)
{
    if (visiting.count(name))
    {
        throw InterpreterError("Circular reference detected: " + name);
    }
    visiting.insert(name);

    std::shared_ptr<Value> found_value = nullptr;

    // Try to find the reference target
    if (defines.count(name))
    {
        found_value = defines.at(name);
    }
    else
    {
        size_t dot_pos = name.find('.');
        if (dot_pos != std::string::npos)
        {
            std::string section_name = name.substr(0, dot_pos);
            std::string key_name = name.substr(dot_pos + 1);
            if (sections.count(section_name) && sections.at(section_name).entries.count(key_name))
            {
                found_value = sections.at(section_name).entries.at(key_name);
            }
        }
    }

    if (!found_value)
    {
        visiting.erase(name); // Backtrack before throwing
        throw InterpreterError("Unresolved reference: " + name);
    }

    // Recursively resolve the found value
    auto resolved_value = resolveValue(found_value, visiting);

    visiting.erase(name); // Backtrack after successful resolution
    return resolved_value;
}

std::shared_ptr<Value> Interpreter::resolveValue(std::shared_ptr<Value> value, std::set<std::string>& visiting)
{
    if (value->isReference())
    {
        return resolveReference(value->asString(), visiting);
    }
    else if (value->isArray() || value->isList())
    {
        Value::ArrayType resolved_arr;
        for (const auto& elem : value->asArray())
        {
            resolved_arr.push_back(resolveValue(elem, visiting));
        }
        return value->isList() ? Value::makeList(resolved_arr) : std::make_shared<Value>(resolved_arr);
    }
    else if (value->isMap())
    {
        Value::MapType resolved_map;
        for (const auto& [key, val] : value->asMap())
        {
            resolved_map[key] = resolveValue(val, visiting);
        }
        return std::make_shared<Value>(resolved_map);
    }

    return value;
}

} // namespace yini