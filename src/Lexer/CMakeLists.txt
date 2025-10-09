# Create a library for the Lexer
add_library(Lexer lexer.cpp)

set_target_properties(Lexer PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Publicly expose the header files to other targets
target_include_directories(Lexer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})