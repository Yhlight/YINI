/**
 * @file JsonDeserializer.hpp
 * @brief Defines the deserializer for converting a JSON string to a
 * YiniDocument.
 */

#ifndef YINI_JSON_DESERIALIZER_HPP
#define YINI_JSON_DESERIALIZER_HPP

#include "YiniData.hpp"
#include <string>

namespace YINI
{
/**
 * @class JsonDeserializer
 * @brief Provides functionality to deserialize a JSON string into a
 * YiniDocument.
 * @details This class is used to load `.ymeta` cache files, which are
 *          stored as JSON, back into the in-memory YiniDocument object model.
 */
class JsonDeserializer
{
public:
  /**
   * @brief Deserializes a JSON string into a YiniDocument.
   * @param json_content The JSON string to deserialize.
   * @param[out] doc The YiniDocument to populate with the deserialized data.
   * @return True if deserialization is successful, false otherwise.
   */
  static bool deserialize(const std::string &json_content,
                          YiniDocument &doc);
};
} // namespace YINI

#endif // YINI_JSON_DESERIALIZER_HPP