#include "yini_c_api.h"
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "Parser/Ast.h"
#include <vector>
#include <string>

extern "C" {

YINI_API void* yini_load_from_string(const char* yini_string)
{
    std::string source(yini_string);
    YINI::Lexer lexer(source);
    std::vector<YINI::Token> tokens = lexer.scanTokens();
    YINI::Parser parser(tokens);
    std::vector<std::unique_ptr<YINI::Stmt>>* ast = new std::vector<std::unique_ptr<YINI::Stmt>>(parser.parse());
    return static_cast<void*>(ast);
}

YINI_API void yini_free_ast(void* ast_ptr)
{
    if (ast_ptr)
    {
        delete static_cast<std::vector<std::unique_ptr<YINI::Stmt>>*>(ast_ptr);
    }
}

}