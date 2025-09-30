add_executable(yini_benchmark parser_benchmark.cpp)

# Link against our library and Google Benchmark
target_link_libraries(yini_benchmark PRIVATE yini benchmark::benchmark)

# Add the benchmark executable as a test so it can be run via ctest
# This is useful for ensuring it builds and runs without error in CI
add_test(
    NAME YiniBenchmark
    COMMAND yini_benchmark
)