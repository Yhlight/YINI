#include <benchmark/benchmark.h>
#include "YINI/Yini.h"
#include <fstream>
#include <streambuf>
#include <string>

// Helper to read the benchmark file content
static std::string readBenchmarkFile() {
    // Note: The benchmark executable runs from the build/benchmarks directory.
    std::ifstream t("../tests/benchmark.yini");
    if (!t.is_open()) {
        // Fallback for different execution environments
        t.open("tests/benchmark.yini");
    }
    std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
    return str;
}

// Define the benchmark case for the yini_parse function
static void BM_YiniParse(benchmark::State& state) {
    std::string yiniContent = readBenchmarkFile();
    if (yiniContent.empty()) {
        state.SkipWithError("Could not read benchmark.yini file. Make sure it exists in the tests/ directory.");
        return;
    }

    // This loop is the core of the benchmark. It will run the code inside it
    // until a statistically stable measurement is achieved.
    for (auto _ : state) {
        YiniDocumentHandle* doc = yini_parse(yiniContent.c_str(), nullptr, 0);
        // We must free the document in each iteration to accurately measure
        // both parsing and destruction time, and to avoid memory leaks.
        if (doc) {
            yini_free_document(doc);
        }
    }
}

// Register the function as a benchmark
BENCHMARK(BM_YiniParse);

// Run the benchmark
BENCHMARK_MAIN();