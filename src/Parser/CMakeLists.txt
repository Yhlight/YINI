add_library(Parser Parser.h Parser.cpp)

target_include_directories(Parser PUBLIC ${CMAKE_SOURCE_DIR}/src)