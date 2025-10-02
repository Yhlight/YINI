add_library(Yini SHARED
    Lexer/Lexer.cpp
    Parser/Parser.cpp
    Parser/AstPrinter.cpp
    Interpreter/Environment.cpp
    Interpreter/Interpreter.cpp
    Core/YiniManager.cpp
    Core/Serialization/Serializer.cpp
    Core/Serialization/Deserializer.cpp
    Interop/YiniCApi.cpp
)

target_include_directories(Yini PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(yini-cli CLI/main.cpp)
target_link_libraries(yini-cli Yini)