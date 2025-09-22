cmake_minimum_required(VERSION 3.16)
project(YINI)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTING "Build the tests" ON)

enable_testing()

add_subdirectory(src)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
