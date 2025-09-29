#ifndef YINI_JSON_DESERIALIZER_HPP
#define YINI_JSON_DESERIALIZER_HPP

#include "YiniData.hpp"
#include <string>

namespace YINI
{
class JsonDeserializer
{
  public:
    // Deserializes a JSON string into a YiniDocument.
    // Returns true on success, false on failure.
    static bool deserialize(const std::string &json_content, YiniDocument &doc);
};
} // namespace YINI

#endif // YINI_JSON_DESERIALIZER_HPP