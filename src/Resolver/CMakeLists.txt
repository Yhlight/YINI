add_library(Resolver Resolver.h Resolver.cpp SemanticInfoVisitor.h SemanticInfoVisitor.cpp)

set_target_properties(Resolver PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(Resolver PRIVATE YmetaManager)

target_include_directories(Resolver PUBLIC ${CMAKE_SOURCE_DIR}/src ${NLOHMANN_JSON_INCLUDE_DIR})