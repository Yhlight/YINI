#include "yini.h"
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "Resolver/Resolver.h"
#include "Evaluator/Evaluator.h"
#include <fstream>
#include <sstream>
#include <map>

// The concrete implementation of the opaque handle
struct YiniDocument
{
    std::unique_ptr<YINI::AST::Program> program;
    // We can also store a map of sections for faster lookups
    std::map<std::string, YINI::AST::Section*> section_map;
};

// Helper to build the section map for fast lookups
void buildSectionMap(YiniDocument* doc)
{
    if (!doc || !doc->program) return;
    for (const auto& stmt : doc->program->statements)
    {
        if (auto* section = dynamic_cast<YINI::AST::Section*>(stmt.get()))
        {
            doc->section_map[section->name->value] = section;
        }
    }
}

YiniResult Yini_LoadFromFile(const char* filepath, YiniDocumentHandle* out_handle)
{
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        return YINI_ERROR_FILE_NOT_FOUND;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    YINI::Lexer lexer(content);
    YINI::Parser parser(lexer);
    auto program = parser.parseProgram();

    if (!parser.getErrors().empty())
    {
        // For simplicity, we just return a generic parse error.
        // A real library might have a way to retrieve detailed error messages.
        return YINI_ERROR_PARSE_ERROR;
    }

    // Resolve macros (placeholder implementation for now)
    YINI::Resolver resolver(*program);
    resolver.resolve();

    // Create the document handle
    YiniDocument* doc = new YiniDocument();
    doc->program = std::move(program);
    buildSectionMap(doc);

    *out_handle = doc;
    return YINI_OK;
}

void Yini_Free(YiniDocumentHandle handle)
{
    if (handle)
    {
        delete handle;
    }
}

YiniResult Yini_GetValue_Int(YiniDocumentHandle handle, const char* key, int64_t* out_value)
{
    if (!handle || !key || !out_value)
    {
        return YINI_ERROR_UNKNOWN; // Should be INVALID_ARG
    }

    std::string key_str(key);
    size_t dot_pos = key_str.find('.');
    if (dot_pos == std::string::npos)
    {
        return YINI_ERROR_KEY_NOT_FOUND; // Invalid key format
    }

    std::string section_name = key_str.substr(0, dot_pos);
    std::string value_key = key_str.substr(dot_pos + 1);

    if (handle->section_map.find(section_name) == handle->section_map.end())
    {
        return YINI_ERROR_KEY_NOT_FOUND;
    }

    YINI::AST::Section* section = handle->section_map.at(section_name);
    YINI::AST::Expression* expression_to_eval = nullptr;

    for (const auto& stmt : section->statements)
    {
        if (auto* kvp = dynamic_cast<YINI::AST::KeyValuePair*>(stmt.get()))
        {
            if (kvp->key->value == value_key)
            {
                expression_to_eval = kvp->value.get();
                break;
            }
        }
    }

    if (!expression_to_eval)
    {
        return YINI_ERROR_KEY_NOT_FOUND;
    }

    YINI::Evaluator evaluator;
    auto result_obj = evaluator.evaluate(expression_to_eval);

    if (result_obj->type != YINI::YiniObject::Type::INTEGER)
    {
        return YINI_ERROR_TYPE_MISMATCH;
    }

    *out_value = std::get<int64_t>(result_obj->value);
    return YINI_OK;
}
