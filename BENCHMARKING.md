# YINI Benchmarking Guide

This document explains how to build and run the performance benchmarks for the YINI project. The project uses [Google Benchmark](https://github.com/google/benchmark) for the C++ core and [BenchmarkDotNet](https://github.com/dotnet/BenchmarkDotNet) for the C# wrapper.

## Running the Benchmarks

The easiest way to run all benchmarks is to use the provided `build.py` script.

From the root of the project, run the following command:

```bash
./build.py benchmark
```

This command will:
1.  Configure the CMake project with benchmarks enabled (`-DYINI_ENABLE_BENCHMARKS=ON`).
2.  Build the entire project in **Release** mode, which is crucial for accurate performance measurements.
3.  Execute the C++ benchmarks.
4.  Execute the C# benchmarks.

### C++ Benchmarks

The C++ benchmarks are built into an executable named `yini_benchmarks` located in the `build/benchmarks/` directory. The output will look similar to this:

```
------------------------------------------------------------
Benchmark               Time             CPU   Iterations
------------------------------------------------------------
BM_YiniParser       15234 ns        15227 ns        45913
```

- **Time:** The real-world time taken per iteration.
- **CPU:** The CPU time taken per iteration.
- **Iterations:** The number of times the benchmarked function was run.

### C# Benchmarks

The C# benchmarks are run using `dotnet run` on the `Yini.Benchmarks` project. BenchmarkDotNet produces a detailed summary table, including memory allocation information:

```
|        Method |     Mean |   Error |  StdDev |  Gen 0 | Allocated |
|-------------- |---------:|--------:|--------:|-------:|----------:|
|  Get<string>  | 24.36 ns | 0.12 ns | 0.11 ns |      - |         - |
|    Get<int>   | 18.91 ns | 0.08 ns | 0.07 ns |      - |         - |
| Get<double>   | 18.95 ns | 0.09 ns | 0.08 ns |      - |         - |
| Get<List...>> | 65.43 ns | 0.33 ns | 0.31 ns | 0.0114 |      96 B |
```

- **Mean:** The average time taken to execute the method.
- **Allocated:** The amount of memory allocated on the managed heap per operation. This is crucial for identifying performance issues related to memory pressure.

## Adding New Benchmarks

### C++
1.  Open the `benchmarks/` directory.
2.  Add a new `BM_MyNewBenchmark` function to an existing `.cpp` file or create a new one.
3.  Register it with the `BENCHMARK()` macro.
4.  If you created a new file, add it to the `add_executable` command in `benchmarks/CMakeLists.txt`.

### C#
1.  Open the `csharp/Yini.Benchmarks/` directory.
2.  Add a new public method annotated with the `[Benchmark]` attribute to the `GetBenchmarks` class (or create a new benchmark class).
3.  The `dotnet run` command will automatically discover and run the new benchmark.