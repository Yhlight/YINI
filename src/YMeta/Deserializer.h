#pragma once

#include "../Parser/AST.h"
#include "YMeta.h"
#include <istream>
#include <memory>

namespace YINI
{
    class Deserializer
    {
    public:
        Deserializer(std::istream& in);
        std::unique_ptr<YiniFile> deserialize();

    private:
        void read(char* data, std::size_t size);
        YMeta::Tag readTag();
        std::string readString();
        std::unique_ptr<Value> readValue();

        std::istream& m_in;
    };
}
