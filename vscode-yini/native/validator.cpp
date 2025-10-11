#include <napi.h>
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "Resolver/Resolver.h"
#include "Validator/Validator.h"
#include "Ymeta/YmetaManager.h"
#include <vector>
#include <string>

// This function is the entry point for the native addon. It takes a string of YINI source code
// and returns an error message if validation fails, or an empty string if it succeeds.
Napi::String ValidateYiniSource(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "String expected").ThrowAsJavaScriptException();
        return Napi::String::New(env, "");
    }

    Napi::String source = info[0].As<Napi::String>();
    std::string source_str = source.Utf8Value();
    std::string error_message = "";

    try {
        YINI::Lexer lexer(source_str);
        auto tokens = lexer.scan_tokens();
        YINI::Parser parser(tokens);
        auto ast = parser.parse();
        YINI::YmetaManager ymeta_manager;
        YINI::Resolver resolver(ast, ymeta_manager);
        auto config = resolver.resolve();
        YINI::Validator validator(config, ast);
        validator.validate();
    } catch (const std::exception& e) {
        error_message = e.what();
    }

    return Napi::String::New(env, error_message);
}

// This function registers the `validateYiniSource` function with the Node.js runtime.
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set(Napi::String::New(env, "validateYiniSource"),
                Napi::Function::New(env, ValidateYiniSource));
    return exports;
}

NODE_API_MODULE(yini_validator, Init)
