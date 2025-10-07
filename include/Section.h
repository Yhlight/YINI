#ifndef YINI_SECTION_H
#define YINI_SECTION_H

#include "Value.h"
#include <string>
#include <vector>
#include <map>
#include <memory>

namespace yini
{

// Represents a configuration section after interpretation
struct Section
{
    std::string name;
    std::vector<std::string> inherited_sections;
    std::map<std::string, std::shared_ptr<Value>> entries;

    Section(const std::string& name = "") : name(name) {}
};

} // namespace yini

#endif // YINI_SECTION_H