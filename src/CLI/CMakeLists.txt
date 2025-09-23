add_executable(yini_cli
    main.cpp
)

# Link the CLI against the core library
target_link_libraries(yini_cli PRIVATE yini_core)
