# This file is only included if the compiler is Clang or GCC.

# Create the fuzzer executable
add_executable(yini_fuzzer fuzzer.cpp)

# Link against our library
target_link_libraries(yini_fuzzer PRIVATE yini)

# Add the necessary compiler flags for libFuzzer and AddressSanitizer
target_compile_options(yini_fuzzer PRIVATE -fsanitize=fuzzer,address)
target_link_libraries(yini_fuzzer PRIVATE -fsanitize=fuzzer,address)