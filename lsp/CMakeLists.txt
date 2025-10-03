# Define the LSP server executable
add_executable(yini-lsp
    main.cpp
)

# Link against the Yini library to get access to the parser and interpreter
target_link_libraries(yini-lsp
    PRIVATE
    Yini
)

# Add the LSP server to the "Apps" folder in IDEs for better organization
set_property(TARGET yini-lsp PROPERTY FOLDER "Apps")

# Install the LSP executable alongside the other tools
install(TARGETS yini-lsp DESTINATION ${CMAKE_INSTALL_BINDIR})