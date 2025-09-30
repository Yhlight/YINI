/**
 * @file JsonSerializer.hpp
 * @brief Defines the serializer for converting a YiniDocument to a JSON string.
 */

#ifndef YINI_JSON_SERIALIZER_HPP
#define YINI_JSON_SERIALIZER_HPP

#include "YiniData.hpp"
#include <string>

namespace YINI
{
/**
 * @class JsonSerializer
 * @brief Provides functionality to serialize a YiniDocument into a JSON format.
 * @details This class is used to create the `.ymeta` cache files, which are
 *          stored as JSON for fast parsing and interoperability.
 */
class JsonSerializer
{
public:
  /**
   * @brief Serializes a YiniDocument into a JSON string.
   * @param doc The YiniDocument to serialize.
   * @return A string containing the JSON representation of the document.
   */
  static std::string serialize(const YiniDocument &doc);
};
} // namespace YINI

#endif // YINI_JSON_SERIALIZER_HPP