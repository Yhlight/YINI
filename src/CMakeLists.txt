# Define the yini library
add_library(yini
    YiniParser.cpp
)

# Publicly expose the include directory for this library
target_include_directories(yini PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)