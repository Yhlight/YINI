add_library(Repl STATIC repl.cpp)
target_include_directories(Repl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Repl PUBLIC YmetaManager)

add_executable(yini main.cpp)
target_link_libraries(yini PRIVATE Repl)

install(TARGETS yini DESTINATION bin)