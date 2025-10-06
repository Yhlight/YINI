# Fetch nlohmann/json
include(FetchContent)
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(json)

# YINI LSP Server executable
add_executable(yini_lsp
    main.cpp
    JSONRPCHandler.cpp
    LSPServer.cpp
    DocumentManager.cpp
    CompletionProvider.cpp
    HoverProvider.cpp
    DefinitionProvider.cpp
    SymbolProvider.cpp
    ReferenceProvider.cpp
    RenameProvider.cpp
    FormattingProvider.cpp
    SemanticTokensProvider.cpp
    WorkspaceSymbolProvider.cpp
)

target_include_directories(yini_lsp PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(yini_lsp PRIVATE
    yini_parser
    yini_lexer
    nlohmann_json::nlohmann_json
)

# Install LSP server
install(TARGETS yini_lsp DESTINATION bin)
