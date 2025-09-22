#ifndef YINI_AST_H
#define YINI_AST_H

#include "Value.h"
#include <string>
#include <vector>
#include <map>

namespace YINI
{

struct KeyValuePair
{
    std::string key;
    Value value;
};

struct Section
{
    std::string name;
    std::vector<std::string> inherited_sections;
    std::vector<KeyValuePair> pairs;
    std::vector<Value> anonymous_values; // For += syntax
};

struct Document
{
    std::map<std::string, Value> defines;
    std::vector<std::string> includes;
    std::vector<Section> sections;
};

} // namespace YINI

#endif // YINI_AST_H
