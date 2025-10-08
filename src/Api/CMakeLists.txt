add_library(yini_shared SHARED
    YiniApi.cpp
    YiniApi.h
)

target_link_libraries(yini_shared PRIVATE yini_core)

target_include_directories(yini_shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)