add_library(yini_loader
    Loader.cpp
    Loader.h
)

target_include_directories(yini_loader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(yini_loader PUBLIC yini_resolver)