#ifndef YINI_VALUE_TO_STRING_HPP
#define YINI_VALUE_TO_STRING_HPP

#include "YINI/YiniData.hpp"
#include <string>

namespace YINI
{
    /**
     * @brief Converts a YiniValue object into its string representation in YINI format.
     * @param value The YiniValue to convert.
     * @return A string representing the value in YINI syntax.
     */
    std::string valueToString(const YiniValue& value);
}

#endif // YINI_VALUE_TO_STRING_HPP