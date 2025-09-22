#ifndef YINI_JSON_H
#define YINI_JSON_H

#include "Parser/AST.h"
#include <string>

namespace YINI
{

class Json
{
public:
    static std::string to_json(const Document& doc);
    // from_json would be more complex, let's focus on serialization first.

private:
    static std::string to_json_value(const Value& value);
};

} // namespace YINI

#endif // YINI_JSON_H
