# CLI application
add_executable(yini_cli
    CLI.cpp
    main.cpp
)

target_link_libraries(yini_cli PRIVATE yini_parser yini_lexer)

target_include_directories(yini_cli PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

target_compile_features(yini_cli PRIVATE cxx_std_17)

# Install CLI executable
install(TARGETS yini_cli
    RUNTIME DESTINATION bin
)
