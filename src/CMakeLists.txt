add_library(yini SHARED
    YiniData.cpp
    Lexer/Lexer.cpp
    Parser/Parser.cpp
    Yini.cpp
    JsonSerializer.cpp
    YiniManager.cpp
    JsonDeserializer.cpp
    YiniValueToString.cpp
    YiniFormatter.cpp
)

target_include_directories(yini PUBLIC ../include)

# Define YINI_EXPORTS when building the shared library
target_compile_definitions(yini PRIVATE YINI_EXPORTS)

target_link_libraries(yini PRIVATE nlohmann_json::nlohmann_json stdc++fs)

# Add coverage flags for Debug builds on GCC/Clang
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(yini PRIVATE --coverage)
        target_link_libraries(yini PRIVATE --coverage)
    endif()
endif()

# Install the library and its public headers
install(TARGETS yini DESTINATION lib)
install(DIRECTORY ../include/YINI DESTINATION include)
