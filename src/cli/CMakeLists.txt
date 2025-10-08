add_executable(yini main.cpp)

target_link_libraries(yini PRIVATE YiniParser)

install(TARGETS yini DESTINATION bin)