add_executable(yini main.cpp)

target_link_libraries(yini PRIVATE Lexer Parser Resolver Validator YmetaManager lz4_static)

target_include_directories(yini PRIVATE ${CMAKE_SOURCE_DIR}/src ${LZ4_INCLUDE_DIR})