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
)
install(
  DIRECTORY src/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Yini
  FILES_MATCHING PATTERN "*.h"
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