#include <benchmark/benchmark.h>
#include <fstream>
#include <sstream>
#include "Core/YiniManager.h"

// A sample YINI file content for benchmarking
const std::string SAMPLE_YINI_CONTENT = R"yini(
[#schema]
[Player]
name: string!
level: integer!
health: float!
is_active: boolean
inventory: [string]
skills: {string: integer}

[Warrior] : Player
strength: integer!
rage: float

[#define]
player_name = "BenchmarkHero"
base_health = 100.0

[Player_1] : Warrior
name = @player_name
level = 50
health = @base_health + 25.5
is_active = true
strength = 99
rage = 55.0
inventory = ["Sword of Benchmarking", "Shield of Performance"]
skills = { "slash": 10, "parry": 8, "charge": 5 }
)yini";

// Define the benchmark for the YINI parser
static void BM_YiniParser(benchmark::State& state) {
  for (auto _ : state) {
    // In each iteration, create a new manager and load the content from the string
    YINI::YiniManager manager;
    manager.load_from_string(SAMPLE_YINI_CONTENT, "benchmark.yini");
  }
}
// Register the function as a benchmark
BENCHMARK(BM_YiniParser);

// It's convention to have a BENCHMARK_MAIN in one of the benchmark files.
// Since this is the first and only one, we'll put it here.
// The benchmark::benchmark_main target already includes this, so we don't need
// to define our own main() function.
// BENCHMARK_MAIN(); // This is handled by benchmark::benchmark_main