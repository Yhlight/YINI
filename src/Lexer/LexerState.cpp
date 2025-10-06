#include "LexerState.h"
#include "Lexer.h"

namespace yini
{

// DefaultState implementation
std::unique_ptr<LexerState> DefaultState::process(Lexer& lexer, char ch)
{
    // Skip whitespace
    if (lexer.isWhitespace(ch))
    {
        return nullptr; // Stay in default state
    }
    
    // Handle newlines
    if (ch == '\n')
    {
        lexer.startBuffer();
        lexer.addToBuffer(ch);
        lexer.emitToken(TokenType::NewLine);
        return nullptr;
    }
    
    // Handle comments
    if (ch == '/')
    {
        char next = lexer.peek();
        if (next == '/')
        {
            lexer.startBuffer();
            lexer.addToBuffer(ch);
            return std::make_unique<CommentState>();
        }
        else if (next == '*')
        {
            lexer.startBuffer();
            lexer.addToBuffer(ch);
            return std::make_unique<BlockCommentState>();
        }
        else
        {
            lexer.startBuffer();
            lexer.addToBuffer(ch);
            lexer.emitToken(TokenType::Slash);
            return nullptr;
        }
    }
    
    // Handle sections
    if (ch == '[')
    {
        lexer.startBuffer();
        lexer.addToBuffer(ch);
        return std::make_unique<SectionState>();
    }
    
    // Handle strings
    if (ch == '"')
    {
        lexer.startBuffer();
        lexer.addToBuffer(ch);
        return std::make_unique<StringState>();
    }
    
    // Handle numbers
    if (lexer.isDigit(ch))
    {
        lexer.startBuffer();
        lexer.addToBuffer(ch);
        return std::make_unique<NumberState>();
    }
    
    // Handle identifiers
    if (lexer.isAlpha(ch))
    {
        lexer.startBuffer();
        lexer.addToBuffer(ch);
        return std::make_unique<IdentifierState>();
    }
    
    // Handle references
    if (ch == '@' || ch == '$')
    {
        lexer.startBuffer();
        lexer.addToBuffer(ch);
        return std::make_unique<ReferenceState>();
    }
    
    // Handle single-character tokens
    lexer.startBuffer();
    lexer.addToBuffer(ch);
    
    switch (ch)
    {
        case '+':
            if (lexer.peek() == '=')
            {
                lexer.addToBuffer(lexer.advance());
                lexer.emitToken(TokenType::PlusAssign);
            }
            else
            {
                lexer.emitToken(TokenType::Plus);
            }
            break;
        case '-':
            lexer.emitToken(TokenType::Minus);
            break;
        case '*':
            lexer.emitToken(TokenType::Star);
            break;
        case '%':
            lexer.emitToken(TokenType::Percent);
            break;
        case '=':
            lexer.emitToken(TokenType::Assign);
            break;
        case ':':
            lexer.emitToken(TokenType::Colon);
            break;
        case ',':
            lexer.emitToken(TokenType::Comma);
            break;
        case '.':
            lexer.emitToken(TokenType::Dot);
            break;
        case '#':
            lexer.emitToken(TokenType::Hash);
            break;
        case ']':
            lexer.emitToken(TokenType::RightBracket);
            break;
        case '(':
            lexer.emitToken(TokenType::LeftParen);
            break;
        case ')':
            lexer.emitToken(TokenType::RightParen);
            break;
        case '{':
            lexer.emitToken(TokenType::LeftBrace);
            break;
        case '}':
            lexer.emitToken(TokenType::RightBrace);
            break;
        default:
            lexer.emitToken(TokenType::Unknown);
            lexer.reportError(std::string("Unexpected character: ") + ch);
            break;
    }
    
    return nullptr;
}

// IdentifierState implementation
std::unique_ptr<LexerState> IdentifierState::process(Lexer& lexer, char ch)
{
    if (lexer.isAlphaNumeric(ch))
    {
        lexer.addToBuffer(ch);
        return nullptr; // Stay in identifier state
    }
    else
    {
        // End of identifier, emit token
        TokenType type = lexer.identifyKeyword(lexer.getBuffer());
        
        if (type == TokenType::True || type == TokenType::False)
        {
            lexer.emitToken(TokenType::Boolean);
        }
        else
        {
            lexer.emitToken(type);
        }
        
        // Put back the character that ended the identifier so it can be processed by DefaultState
        if (ch != '\0')
        {
            lexer.unget();
        }
        
        return std::make_unique<DefaultState>();
    }
}

// NumberState implementation
std::unique_ptr<LexerState> NumberState::process(Lexer& lexer, char ch)
{
    if (lexer.isDigit(ch))
    {
        lexer.addToBuffer(ch);
        return nullptr;
    }
    else if (ch == '.' && lexer.getBuffer().find('.') == std::string::npos)
    {
        // First decimal point
        lexer.addToBuffer(ch);
        return nullptr;
    }
    else
    {
        // End of number
        if (lexer.getBuffer().find('.') != std::string::npos)
        {
            lexer.emitToken(TokenType::Float);
        }
        else
        {
            lexer.emitToken(TokenType::Integer);
        }
        
        // Put back the character that ended the number so it can be processed by DefaultState
        if (ch != '\0')
        {
            lexer.unget();
        }
        
        return std::make_unique<DefaultState>();
    }
}

// StringState implementation
std::unique_ptr<LexerState> StringState::process(Lexer& lexer, char ch)
{
    lexer.addToBuffer(ch);
    
    if (ch == '"' && lexer.getBuffer().length() > 1)
    {
        // End of string
        std::string buffer = lexer.getBuffer();
        std::string value = buffer.substr(1, buffer.length() - 2); // Remove quotes
        lexer.emitToken(TokenType::String, value);
        return std::make_unique<DefaultState>();
    }
    else if (ch == '\n' || lexer.isAtEnd())
    {
        lexer.reportError("Unterminated string literal");
        lexer.emitToken(TokenType::Unknown);
        return std::make_unique<DefaultState>();
    }
    
    return nullptr; // Stay in string state
}

// CommentState implementation
std::unique_ptr<LexerState> CommentState::process(Lexer& lexer, char ch)
{
    if (ch == '\n' || lexer.isAtEnd())
    {
        // End of line comment - don't include the newline
        lexer.emitToken(TokenType::Comment);
        
        // Put back the newline so DefaultState can process it
        if (ch == '\n')
        {
            lexer.unget();
        }
        
        return std::make_unique<DefaultState>();
    }
    
    lexer.addToBuffer(ch);
    return nullptr; // Stay in comment state
}

// BlockCommentState implementation
std::unique_ptr<LexerState> BlockCommentState::process(Lexer& lexer, char ch)
{
    lexer.addToBuffer(ch);
    
    if (ch == '*' && lexer.peek() == '/')
    {
        lexer.addToBuffer(lexer.advance());
        lexer.emitToken(TokenType::Comment);
        return std::make_unique<DefaultState>();
    }
    else if (lexer.isAtEnd())
    {
        lexer.reportError("Unterminated block comment");
        lexer.emitToken(TokenType::Comment);
        return std::make_unique<DefaultState>();
    }
    
    return nullptr; // Stay in block comment state
}

// SectionState implementation
std::unique_ptr<LexerState> SectionState::process(Lexer& lexer, char ch)
{
    lexer.addToBuffer(ch);
    
    if (ch == ']')
    {
        // End of section header
        std::string buffer = lexer.getBuffer();
        
        // Check for empty section []
        if (buffer == "[]")
        {
            // Emit as separate delimiters
            lexer.clearBuffer();
            lexer.startBuffer();
            lexer.addToBuffer('[');
            lexer.emitToken(TokenType::LeftBracket);
            lexer.startBuffer();
            lexer.addToBuffer(']');
            lexer.emitToken(TokenType::RightBracket);
            return std::make_unique<DefaultState>();
        }
        
        // Check for special sections
        if (buffer.find("#define") != std::string::npos)
        {
            lexer.emitToken(TokenType::Define);
        }
        else if (buffer.find("#include") != std::string::npos)
        {
            lexer.emitToken(TokenType::Include);
        }
        else if (buffer.find("#schema") != std::string::npos)
        {
            lexer.emitToken(TokenType::Schema);
        }
        else
        {
            // Extract section name
            std::string value = buffer.substr(1, buffer.length() - 2);
            lexer.emitToken(TokenType::Section, value);
        }
        
        return std::make_unique<DefaultState>();
    }
    else if (ch == '\n' || lexer.isAtEnd())
    {
        lexer.reportError("Unterminated section header");
        lexer.emitToken(TokenType::Unknown);
        return std::make_unique<DefaultState>();
    }
    
    return nullptr; // Stay in section state
}

// ReferenceState implementation
std::unique_ptr<LexerState> ReferenceState::process(Lexer& lexer, char ch)
{
    std::string buffer = lexer.getBuffer();
    
    // Environment variable ${name}
    if (buffer[0] == '$' && buffer.length() == 1 && ch == '{')
    {
        lexer.addToBuffer(ch);
        return nullptr;
    }
    else if (buffer[0] == '$' && buffer.find('{') != std::string::npos)
    {
        if (ch == '}')
        {
            lexer.addToBuffer(ch);
            std::string value = buffer.substr(2, buffer.length() - 2); // Remove ${ }
            lexer.emitToken(TokenType::EnvVar, value);
            return std::make_unique<DefaultState>();
        }
        else if (lexer.isAlphaNumeric(ch) || ch == '_')
        {
            lexer.addToBuffer(ch);
            return nullptr;
        }
        else
        {
            lexer.reportError("Invalid environment variable reference");
            lexer.emitToken(TokenType::Unknown);
            return std::make_unique<DefaultState>();
        }
    }
    // Cross-section reference @{Section.key}
    else if (buffer[0] == '@' && buffer.length() == 1 && ch == '{')
    {
        lexer.addToBuffer(ch);
        return nullptr;
    }
    else if (buffer[0] == '@' && buffer.find('{') != std::string::npos)
    {
        if (ch == '}')
        {
            lexer.addToBuffer(ch);
            std::string value = buffer.substr(2, buffer.length() - 2); // Remove @{ }
            lexer.emitToken(TokenType::CrossRef, value);
            return std::make_unique<DefaultState>();
        }
        else if (lexer.isAlphaNumeric(ch) || ch == '_' || ch == '.')
        {
            lexer.addToBuffer(ch);
            return nullptr;
        }
        else
        {
            lexer.reportError("Invalid cross-section reference");
            lexer.emitToken(TokenType::Unknown);
            return std::make_unique<DefaultState>();
        }
    }
    // Macro reference @name
    else if (buffer[0] == '@' && (lexer.isAlphaNumeric(ch) || ch == '_'))
    {
        lexer.addToBuffer(ch);
        return nullptr;
    }
    else
    {
        // End of macro reference
        if (buffer.length() > 1)
        {
            std::string value = buffer.substr(1); // Remove @
            lexer.emitToken(TokenType::MacroRef, value);
        }
        else
        {
            lexer.emitToken(TokenType::At);
        }
        
        // Put back the character that ended the reference
        if (ch != '\0')
        {
            lexer.unget();
        }
        
        return std::make_unique<DefaultState>();
    }
}

} // namespace yini
