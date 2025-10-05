# This CMakeLists.txt file is for the IDE components of YINI.

# Add the Language Server Protocol (LSP) server project.
# This ensures that the LSP server is built as part of the main project.
add_subdirectory(LspServer)