add_library(YiniInterop SHARED yini_interop.cpp)

target_include_directories(YiniInterop PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(YiniInterop PUBLIC YiniParser YmetaManager)

install(TARGETS YiniInterop DESTINATION lib)