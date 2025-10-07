#ifndef YINI_RESOLVER_H
#define YINI_RESOLVER_H

#include "../Parser/Ast.h"

namespace YINI
{
    class Resolver
    {
    public:
        void resolve(AstNode& ast);

    private:
        YiniValue* findValue(AstNode& ast, const std::string& sectionName, const std::string& keyName);
    };
}

#endif // YINI_RESOLVER_H