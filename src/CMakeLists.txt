# Define the YINI library
add_library(yini SHARED)

# Add sources from subdirectories
target_sources(yini
    PRIVATE
        Lexer/Lexer.cpp
        Parser/Parser.cpp
        Parser/AstSerializer.cpp
        yini_c_api.cpp
)

# Define YINI_EXPORTS when building the yini library
target_compile_definitions(yini PRIVATE YINI_EXPORTS)

# Public headers
target_include_directories(yini
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Lexer
        ${CMAKE_CURRENT_SOURCE_DIR}/Parser
)

target_link_libraries(yini PUBLIC nlohmann_json::nlohmann_json)

# CLI executable
add_executable(yini-cli CLI/main.cpp)
target_link_libraries(yini-cli PRIVATE yini)