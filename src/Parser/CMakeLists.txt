add_library(Parser Parser.h Parser.cpp)

set_target_properties(Parser PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(Parser PUBLIC ${CMAKE_SOURCE_DIR}/src)