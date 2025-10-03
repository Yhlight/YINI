cmake_minimum_required(VERSION 3.10)
project(YINI VERSION 1.0.0 LANGUAGES CXX)

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Option to build shared libraries (ON by default)
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # Disable GNU extensions

# Compiler warnings
if(MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Code coverage option (for GCC/Clang)
option(YINI_ENABLE_COVERAGE "Enable code coverage reporting" OFF)
if(YINI_ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            message(STATUS "Code coverage enabled for ${CMAKE_CXX_COMPILER_ID}. Compiler flags added.")
            add_compile_options(--coverage)
            add_link_options(--coverage)
        else()
            message(WARNING "Code coverage is only available for Debug builds. Please use -DCMAKE_BUILD_TYPE=Debug.")
        endif()
    else()
        message(WARNING "Code coverage is only available for GCC and Clang.")
    endif()
endif()

# Enable testing
enable_testing()

# Add subdirectories
add_subdirectory(src)
add_subdirectory(tests)

# Install rules
include(GNUInstallDirs)
install(
  TARGETS Yini
  EXPORT YiniTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Yini
)

# Generate and install CMake package configuration files
include(CMakePackageConfigHelpers)
configure_package_config_file(
  cmake/YiniConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/YiniConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Yini
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/YiniConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)
install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/YiniConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/YiniConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Yini
)
install(
  EXPORT YiniTargets
  FILE YiniTargets.cmake
  NAMESPACE Yini::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Yini
)

# Doxygen integration
find_package(Doxygen)
if(DOXYGEN_FOUND)
  add_custom_target(doc
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM
  )
endif()

# Code coverage target
if(YINI_ENABLE_COVERAGE AND (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang") AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    find_program(LCOV_EXECUTABLE lcov)
    find_program(GENHTML_EXECUTABLE genhtml)
    if(LCOV_EXECUTABLE AND GENHTML_EXECUTABLE)
        add_custom_target(coverage
            # Step 1: Zero out coverage counters
            COMMAND ${LCOV_EXECUTABLE} --zerocounters --directory .
            # Step 2: Run tests to generate coverage data
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            # Step 3: Capture coverage data, ignoring certain errors that can occur with C++ templates
            COMMAND ${LCOV_EXECUTABLE} --capture --directory . --output-file coverage.info --ignore-errors mismatch
            # Step 4: Remove external/test coverage data
            COMMAND ${LCOV_EXECUTABLE} --remove coverage.info /usr/* ${CMAKE_BINARY_DIR}/* */tests/* --output-file coverage.info
            # Step 5: Generate HTML report
            COMMAND ${GENHTML_EXECUTABLE} coverage.info --output-directory coverage_html
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating code coverage report..."
            DEPENDS
                lexer_test
                parser_test
                expression_test
                interpreter_test
                include_test
                dyna_test
                serialization_test
                cli_test
                syntax_error_test
                complex_interpreter_test
                c_api_test
            VERBATIM
        )
    else()
        message(WARNING "lcov/genhtml not found. Coverage target will not be available.")
    endif()
endif()