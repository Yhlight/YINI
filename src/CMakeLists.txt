add_library(yini_core INTERFACE)

target_include_directories(yini_core INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Add subdirectories for different components
add_subdirectory(Lexer)
add_subdirectory(Parser)
add_subdirectory(CLI)
add_subdirectory(Api)
add_subdirectory(Resolver)
add_subdirectory(Loader)

# Link all parts to the core library
target_link_libraries(yini_core INTERFACE yini_lexer yini_parser yini_resolver yini_loader)