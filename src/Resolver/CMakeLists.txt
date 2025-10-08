add_library(yini_resolver
    Resolver.cpp
    Resolver.h
)

target_include_directories(yini_resolver PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(yini_resolver PUBLIC yini_parser)