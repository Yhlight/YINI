#ifndef YINI_ERROR_H
#define YINI_ERROR_H

#include <string>
#include <vector>
#include <sstream>

namespace Yini
{
    enum class ErrorType
    {
        None,
        Parsing,
        Runtime,
        Type,
        System
    };

    struct YiniError
    {
        ErrorType type = ErrorType::None;
        std::string message;
        int line = 0;
        int column = 0;

        YiniError(ErrorType t, std::string msg, int l = 0, int c = 0)
            : type(t), message(std::move(msg)), line(l), column(c) {}

        std::string toString() const
        {
            std::stringstream ss;
            ss << "[";
            switch(type)
            {
                case ErrorType::Parsing: ss << "Parsing"; break;
                case ErrorType::Runtime: ss << "Runtime"; break;
                case ErrorType::Type:    ss << "Type"; break;
                case ErrorType::System:  ss << "System"; break;
                default:                 ss << "Unknown"; break;
            }
            ss << " Error]";
            if (line > 0)
            {
                ss << " (Line " << line << ", Col " << column << ")";
            }
            ss << ": " << message;
            return ss.str();
        }
    };
}

#endif // YINI_ERROR_H
