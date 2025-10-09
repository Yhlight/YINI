add_executable(yini-langserver main.cpp)

target_include_directories(yini-langserver PUBLIC ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(yini-langserver PRIVATE YiniParser nlohmann_json::nlohmann_json)

install(TARGETS yini-langserver DESTINATION bin)