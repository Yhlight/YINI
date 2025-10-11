add_executable(yini-ls main.cpp)

target_link_libraries(yini-ls
    PRIVATE
    Lexer
    Parser
    Resolver
    Validator
    YmetaManager
    nlohmann_json
)

target_include_directories(yini-ls PUBLIC ${CMAKE_SOURCE_DIR}/src)
