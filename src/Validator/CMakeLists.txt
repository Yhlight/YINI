add_library(Validator Validator.h Validator.cpp)

set_target_properties(Validator PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(Validator PUBLIC ${CMAKE_SOURCE_DIR}/src)