#pragma once

#include "../Parser/AST.h"
#include "YMeta.h"
#include <ostream>
#include <memory>

namespace YINI
{
    class Serializer
    {
    public:
        Serializer(const YiniFile& yini_file);
        void serialize(std::ostream& out);

    private:
        void write(const char* data, std::size_t size);
        void writeTag(YMeta::Tag tag);
        void writeString(const std::string& str);
        void writeValue(const Value& value);

        const YiniFile& m_yini_file;
        std::ostream* m_out;
    };
}
