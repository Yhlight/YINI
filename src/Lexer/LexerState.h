#ifndef YINI_LEXER_LEXER_STATE_H
#define YINI_LEXER_LEXER_STATE_H

#include "Token.h"
#include <memory>
#include <string>

namespace yini
{

// Forward declaration
class Lexer;

// Base state class for Lexer state machine
class LexerState
{
public:
    virtual ~LexerState() = default;
    
    // Process current character and return next state
    virtual std::unique_ptr<LexerState> process(Lexer& lexer, char ch) = 0;
    
    // Get the state name for debugging
    virtual std::string getName() const = 0;
};

// Default state - initial state
class DefaultState : public LexerState
{
public:
    std::unique_ptr<LexerState> process(Lexer& lexer, char ch) override;
    std::string getName() const override { return "Default"; }
};

// Identifier state - reading identifiers and keywords
class IdentifierState : public LexerState
{
public:
    std::unique_ptr<LexerState> process(Lexer& lexer, char ch) override;
    std::string getName() const override { return "Identifier"; }
};

// Number state - reading numbers (int and float)
class NumberState : public LexerState
{
public:
    std::unique_ptr<LexerState> process(Lexer& lexer, char ch) override;
    std::string getName() const override { return "Number"; }
};

// String state - reading string literals
class StringState : public LexerState
{
public:
    std::unique_ptr<LexerState> process(Lexer& lexer, char ch) override;
    std::string getName() const override { return "String"; }
};

// Comment state - reading comments
class CommentState : public LexerState
{
public:
    std::unique_ptr<LexerState> process(Lexer& lexer, char ch) override;
    std::string getName() const override { return "Comment"; }
};

// Block comment state - reading /* */ comments
class BlockCommentState : public LexerState
{
public:
    std::unique_ptr<LexerState> process(Lexer& lexer, char ch) override;
    std::string getName() const override { return "BlockComment"; }
};

// Section state - reading section headers
class SectionState : public LexerState
{
public:
    std::unique_ptr<LexerState> process(Lexer& lexer, char ch) override;
    std::string getName() const override { return "Section"; }
};

// Reference state - reading macro/env/cross references
class ReferenceState : public LexerState
{
public:
    std::unique_ptr<LexerState> process(Lexer& lexer, char ch) override;
    std::string getName() const override { return "Reference"; }
};

} // namespace yini

#endif // YINI_LEXER_LEXER_STATE_H
