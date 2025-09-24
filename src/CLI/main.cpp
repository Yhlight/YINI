#include <iostream>
#include <string>
#include <vector>
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "Parser/AST.h"
#include "Resolver/Resolver.h"

int main()
{
    std::string input = R"yini(
[#define]
base_damage = 10

[PlayerStats]
// Uses a macro and arithmetic
attack = @base_damage + 5 * 2
health = Dyna(100)
)yini";

    YINI::Lexer lexer(input);
    YINI::Parser parser(lexer);

    auto program = parser.parseProgram();

    if (!parser.getErrors().empty())
    {
        std::cerr << "--- Parser Errors ---" << std::endl;
        for (const auto& error : parser.getErrors())
        {
            std::cerr << error << std::endl;
        }
        return 1;
    }

    std::cout << "--- Parser Test (Before Resolution) ---" << std::endl;
    std::cout << program->toString() << std::endl;

    std::cout << "--- Resolving AST ---" << std::endl;
    YINI::Resolver resolver(*program);
    resolver.resolve();

    // We don't print the tree again because the placeholder doesn't actually modify it.
    // The resolver.resolve() call will print its own message.

    return 0;
}
