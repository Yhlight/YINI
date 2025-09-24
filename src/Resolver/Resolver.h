#pragma once

#include "../Parser/AST.h"
#include <map>
#include <string>

namespace YINI
{
    class Resolver
    {
    public:
        Resolver(AST::Program& program);

        void resolve();

    private:
        void gatherMacros(AST::Node* node);
        void substituteMacros(AST::Node* node);

        AST::Program& m_program;
        std::map<std::string, AST::Expression*> m_macro_table;
    };
}
