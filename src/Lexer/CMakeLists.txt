# Create a library for the Lexer
add_library(Lexer lexer.cpp)

# Publicly expose the header files to other targets
target_include_directories(Lexer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})