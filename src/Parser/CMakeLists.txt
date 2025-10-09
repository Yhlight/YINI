add_library(YiniParser parser.cpp)

set_target_properties(YiniParser PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(YiniParser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(YiniParser PUBLIC Lexer nlohmann_json::nlohmann_json)