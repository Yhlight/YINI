add_library(Resolver Resolver.h Resolver.cpp)

target_include_directories(Resolver PUBLIC ${CMAKE_SOURCE_DIR}/src)