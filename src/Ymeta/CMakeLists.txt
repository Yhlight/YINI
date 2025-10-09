add_library(YmetaManager YmetaManager.cpp)

set_target_properties(YmetaManager PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(YmetaManager PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(YmetaManager PUBLIC YiniParser)