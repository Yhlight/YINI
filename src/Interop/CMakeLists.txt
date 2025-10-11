add_library(YiniInterop SHARED yini_interop.h yini_interop.cpp)

set_target_properties(YiniInterop PROPERTIES
    CXX_STANDARD 17
    POSITION_INDEPENDENT_CODE ON)

target_link_libraries(YiniInterop PRIVATE Lexer Parser Resolver Validator YmetaManager Loader nlohmann_json)

target_include_directories(YiniInterop PUBLIC ${CMAKE_SOURCE_DIR}/src)