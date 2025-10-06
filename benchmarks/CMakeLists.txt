add_executable(yini_benchmarks
    ParserBenchmark.cpp
)

target_link_libraries(yini_benchmarks PRIVATE
    Yini
    benchmark::benchmark_main
)

set_target_properties(yini_benchmarks PROPERTIES
    FOLDER "Benchmarks"
)