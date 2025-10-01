/**
 * @file YiniSerializer.hpp
 * @brief Defines the YiniSerializer class for serializing YiniDocument objects.
 */

#ifndef YINI_SERIALIZER_HPP
#define YINI_SERIALIZER_HPP

#include "YiniData.hpp"
#include <string>

namespace YINI
{

/**
 * @class YiniSerializer
 * @brief Provides functionality to serialize a YiniDocument into a string.
 */
class YiniSerializer
{
public:
    /**
     * @brief Serializes a YiniDocument into the YINI file format.
     * @param document The YiniDocument to serialize.
     * @return A string containing the serialized document in YINI format.
     */
    static std::string serialize(const YiniDocument& document);
};

} // namespace YINI

#endif // YINI_SERIALIZER_HPP