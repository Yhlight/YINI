#pragma once

#include <string>
#include <map>
#include <any>

namespace YINI
{

class Cooker
{
public:
    Cooker();
    void cook(const std::map<std::string, std::any>& config, const std::string& output_path);

private:
    // Private helper methods for serialization will be defined here.
};

} // namespace YINI
