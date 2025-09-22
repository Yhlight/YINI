add_library(YiniLib)

target_include_directories(YiniLib PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/Core"
    "${CMAKE_CURRENT_SOURCE_DIR}/Lexer"
    "${CMAKE_CURRENT_SOURCE_DIR}/Parser"
    "${CMAKE_CURRENT_SOURCE_DIR}/Loader"
    "${CMAKE_CURRENT_SOURCE_DIR}/Serializer"
)

file(GLOB_RECURSE YiniLib_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/Core/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Lexer/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Parser/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Loader/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Serializer/*.cpp"
)

target_sources(YiniLib PRIVATE ${YiniLib_SOURCES})

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(YiniLib PUBLIC stdc++fs)
endif()

add_subdirectory(CLI)
