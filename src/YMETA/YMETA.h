#ifndef YINI_YMETA_H
#define YINI_YMETA_H

#include "../Parser/Ast.h"
#include <iostream>
#include <fstream>

namespace YINI
{
    namespace YMETA
    {
        // Forward declarations
        void serialize(const AstNode& ast, std::ofstream& out);
        std::unique_ptr<AstNode> deserialize(std::ifstream& in);
    }
}

#endif // YINI_YMETA_H