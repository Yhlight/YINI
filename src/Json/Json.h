#pragma once

#include "../Parser/Ast.h"
#include <string>
#include <sstream>

namespace Yini
{
    class JsonWriter
    {
    public:
        static std::string write(const YiniValue& value);

    private:
        static void writeValue(std::stringstream& ss, const YiniValue& value);
    };
}
