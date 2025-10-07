#ifndef YINI_LSP_TYPES_H
#define YINI_LSP_TYPES_H

namespace yini::lsp
{

struct Position
{
    int line;
    int character;
};

struct Range
{
    Position start;
    Position end;
};

} // namespace yini::lsp

#endif // YINI_LSP_TYPES_H
