add_library(Loader STATIC YbinLoader.cpp)

set_target_properties(Loader PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(Loader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
