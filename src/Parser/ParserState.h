#ifndef YINI_PARSER_PARSER_STATE_H
#define YINI_PARSER_PARSER_STATE_H

#include <memory>

namespace yini
{

class Parser;

// Base parser state (placeholder)
class ParserState
{
public:
    virtual ~ParserState() = default;
    virtual void process(Parser& parser) = 0;
};

} // namespace yini

#endif // YINI_PARSER_PARSER_STATE_H
