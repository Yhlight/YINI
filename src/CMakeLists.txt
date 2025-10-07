# Create a library for the YINI parser
add_library(yini)
set_target_properties(yini PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_sources(yini PRIVATE
  yini.cpp
  Lexer/Lexer.cpp
  Parser/Parser.cpp
  Parser/Ast.cpp
  Resolver/Resolver.cpp
  YMETA/YMETA.cpp
)

# Public headers
target_include_directories(yini PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Create the CLI executable
add_executable(yini_cli cli.cpp)

# Link the CLI to the yini library
target_link_libraries(yini_cli yini)

# Create the C API shared library
add_library(yini_c SHARED c_api/yini_c_api.cpp)
target_link_libraries(yini_c PRIVATE yini)
set_target_properties(yini_c PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Define the export macro
target_compile_definitions(yini_c PRIVATE YINI_C_API_EXPORTS)