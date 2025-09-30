#ifndef YINI_FORMATTER_HPP
#define YINI_FORMATTER_HPP

#include "YiniData.hpp"
#include <string>

namespace YINI
{

/**
 * @class YiniFormatter
 * @brief Provides functionality to format a YiniDocument into a standardized, readable string.
 */
class YiniFormatter
{
public:
    /**
     * @brief Formats the given YiniDocument.
     * @param doc The document to format.
     * @return A string containing the formatted YINI data.
     */
    static std::string format(const YiniDocument& doc);
};

} // namespace YINI

#endif // YINI_FORMATTER_HPP