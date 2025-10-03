add_library(Yini
    Lexer/Lexer.cpp
    Parser/Parser.cpp
    Parser/AstPrinter.cpp
    Interpreter/Environment.cpp
    Interpreter/Interpreter.cpp
    Core/YiniManager.cpp
    Core/YiniValue.cpp
    Core/DynaValue.cpp
    Core/Serialization/Serializer.cpp
    Core/Serialization/Deserializer.cpp
    Core/Validator.cpp
    Interop/YiniCApi.cpp
)

target_include_directories(Yini
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

target_sources(Yini
    PUBLIC FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
    FILES
        Core/DynaValue.h
        Core/YiniException.h
        Core/YiniManager.h
        Core/YiniValue.h
        Core/Validator.h
        Interop/YiniCApi.h
)

target_compile_features(Yini PUBLIC cxx_std_17)

option(YINI_BUILD_CLI "Build the YINI command-line interface" ON)

if(YINI_BUILD_CLI)
    add_executable(yini-cli CLI/main.cpp)
    target_link_libraries(yini-cli PRIVATE Yini)
    target_compile_features(yini-cli PRIVATE cxx_std_17)

    install(TARGETS yini-cli DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()