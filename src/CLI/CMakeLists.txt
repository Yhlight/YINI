add_executable(yini main.cpp)

target_link_libraries(yini PRIVATE Lexer Parser)

target_include_directories(yini PRIVATE ${CMAKE_SOURCE_DIR}/src)