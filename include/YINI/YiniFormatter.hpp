#ifndef YINI_FORMATTER_HPP
#define YINI_FORMATTER_HPP

#include "YiniData.hpp"
#include <string>

namespace YINI
{
    /**
     * @brief A utility class to format YiniValue objects back into their string representation.
     */
    class YiniFormatter
    {
    public:
        /**
         * @brief Formats a YiniValue into its YINI string representation.
         * @param value The YiniValue to format.
         * @return A string representing the value in YINI format.
         */
        static std::string format(const YiniValue& value);

        /**
         * @brief Formats an entire YiniDocument into a string.
         * @param doc The YiniDocument to format.
         * @return A string representing the document in YINI format.
         */
        static std::string formatDocument(const YiniDocument& doc);
    };
}

#endif // YINI_FORMATTER_HPP