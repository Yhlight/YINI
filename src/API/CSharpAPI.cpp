#include "CSharpAPI.h"
#include "../Lexer/Lexer.h"
#include <cstring>

extern "C"
{

void* yini_create_lexer(const char* source)
{
    if (!source)
        return nullptr;
    
    std::string src(source);
    return new yini::Lexer(src);
}

void yini_destroy_lexer(void* lexer)
{
    if (lexer)
    {
        delete static_cast<yini::Lexer*>(lexer);
    }
}

int yini_tokenize(void* lexer)
{
    if (!lexer)
        return -1;
    
    auto* lex = static_cast<yini::Lexer*>(lexer);
    auto tokens = lex->tokenize();
    return static_cast<int>(tokens.size());
}

} // extern "C"
