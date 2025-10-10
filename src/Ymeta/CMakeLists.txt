include(FetchContent)
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(nlohmann_json)

add_library(YmetaManager YmetaManager.h YmetaManager.cpp)

set_target_properties(YmetaManager PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(YmetaManager PRIVATE nlohmann_json)
target_include_directories(YmetaManager PUBLIC ${CMAKE_SOURCE_DIR}/src)