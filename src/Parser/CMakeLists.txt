add_library(YiniParser parser.cpp)

target_include_directories(YiniParser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(YiniParser PUBLIC Lexer)