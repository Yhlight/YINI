#ifndef YINI_C_API_INTERNAL_H
#define YINI_C_API_INTERNAL_H

#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "Runtime/Runtime.h"

struct YiniHandleInternal
{
    Yini::YiniRuntime* runtime;
    std::vector<Yini::YiniError> aggregated_errors;
};

#endif // YINI_C_API_INTERNAL_H
