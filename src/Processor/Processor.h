#pragma once

#include "../Parser/AST.h"
#include <memory>

namespace YINI
{
    class Processor
    {
    public:
        Processor(std::unique_ptr<YiniFile> yini_file);

        std::unique_ptr<YiniFile> process();

    private:
        void processIncludes();
        void processInheritance();
        void processMacros();

        std::unique_ptr<YiniFile> m_yini_file;
    };
}
