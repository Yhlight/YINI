#include "YiniApi.h"
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "Parser/Ast.h"
#include "Resolver/Resolver.h"

#include <vector>
#include <memory>

extern "C" {

YINI_API YiniDocumentHandle yini_parse_string(const char* source)
{
    try
    {
        std::string sourceStr(source);
        Yini::Lexer lexer(sourceStr);
        std::vector<Yini::Token> tokens = lexer.scanTokens();
        Yini::Parser parser(tokens);
        auto ast = parser.parse();
        Yini::Resolver resolver(ast);
        resolver.resolve();

        auto* ast_ptr = new std::vector<std::unique_ptr<Yini::SectionNode>>(std::move(ast));
        return reinterpret_cast<YiniDocumentHandle>(ast_ptr);
    }
    catch (...)
    {
        return nullptr;
    }
}

YINI_API void yini_free_document(YiniDocumentHandle handle)
{
    if (handle)
    {
        auto ast = reinterpret_cast<std::vector<std::unique_ptr<Yini::SectionNode>>*>(handle);
        delete ast;
    }
}

} // extern "C"