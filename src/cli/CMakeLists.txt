add_executable(yini main.cpp)

target_link_libraries(yini PRIVATE YmetaManager nlohmann_json::nlohmann_json)

install(TARGETS yini DESTINATION bin)