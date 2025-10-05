# Define a custom target to build the C# LSP Server project.
# This ensures that our `dotnet` build command is executed by CMake.
add_custom_target(LspServer_build ALL
    COMMAND ${dotnet_EXECUTABLE} build ${CMAKE_CURRENT_SOURCE_DIR}/LspServer.csproj
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Building C# LSP Server"
    VERBATIM
)