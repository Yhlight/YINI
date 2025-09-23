#ifndef YINI_C_API_INTERNAL_H
#define YINI_C_API_INTERNAL_H

#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "Runtime/Runtime.h"

struct YiniHandleInternal
{
    Yini::Lexer* lexer;
    Yini::Parser* parser;
    Yini::YiniRuntime* runtime;
};

#endif // YINI_C_API_INTERNAL_H
