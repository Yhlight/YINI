add_library(yini SHARED
    YiniData.cpp
    Lexer/Lexer.cpp
    Parser/Parser.cpp
    Yini.cpp
    JsonSerializer.cpp
    YiniSerializer.cpp
    YiniManager.cpp
    JsonDeserializer.cpp
)

target_include_directories(yini PUBLIC ../include)

# Define YINI_EXPORTS when building the shared library
target_compile_definitions(yini PRIVATE YINI_EXPORTS)

target_link_libraries(yini PRIVATE nlohmann_json::nlohmann_json)
