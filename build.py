#!/usr/bin/env python3

import argparse
import os
import shutil
import subprocess
import sys

def run_command(command, cwd=None, env=None):
    """Runs a command with an optional environment and prints its output in real-time."""
    print(f"Executing: {' '.join(command)}")

    # Combine with the current environment if a custom one is provided
    process_env = os.environ.copy()
    if env:
        process_env.update(env)

    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, cwd=cwd, env=process_env)
    for line in process.stdout:
        print(line, end='')
    process.wait()
    if process.returncode != 0:
        print(f"Error: Command failed with exit code {process.returncode}")
        sys.exit(process.returncode)

def main():
    parser = argparse.ArgumentParser(description="YINI Project Build Script")
    parser.add_argument("action", choices=["build", "test", "docs", "clean", "all", "coverage", "bench"], help="Action to perform")
    parser.add_argument("--config", default="Release", choices=["Release", "Debug"], help="Build configuration")
    args = parser.parse_args()

    project_root = os.path.dirname(os.path.abspath(__file__))
    build_dir = os.path.join(project_root, "build")

    if args.action == "clean":
        print("Cleaning build directory...")
        if os.path.exists(build_dir):
            shutil.rmtree(build_dir)
        print("Clean complete.")
        return

    # Handle coverage action separately as it has special requirements
    if args.action == "coverage":
        print("--- Running Coverage ---")
        if args.config != "Debug":
            print("Warning: Coverage requires Debug build. Overriding configuration.")

        # Coverage requires a clean configure and build
        if os.path.exists(build_dir):
            print("Cleaning build directory for a fresh coverage build...")
            shutil.rmtree(build_dir)

        os.makedirs(build_dir, exist_ok=True)

        print("Configuring for coverage...")
        cmake_configure_command = ["cmake", "-S", ".", "-B", build_dir, "-DCMAKE_BUILD_TYPE=Debug", "-DYINI_ENABLE_COVERAGE=ON"]
        vcpkg_toolchain_file = os.path.join(os.environ.get("VCPKG_ROOT", ""), "scripts/buildsystems/vcpkg.cmake")
        if os.path.exists(vcpkg_toolchain_file):
            cmake_configure_command.append(f"-DCMAKE_TOOLCHAIN_FILE={vcpkg_toolchain_file}")
        run_command(cmake_configure_command)

        print("Building and running coverage target...")
        # The 'coverage' target in CMakeLists.txt builds, runs tests, and generates the report
        run_command(["cmake", "--build", build_dir, "--target", "coverage"])
        print("Coverage report generated in build/coverage_html")
        return

    # --- Standard Actions ---
    build_config = args.config

    # Configure step
    if not os.path.exists(os.path.join(build_dir, "CMakeCache.txt")):
        print("Configuring CMake...")
        os.makedirs(build_dir, exist_ok=True)
        cmake_configure_command = ["cmake", "-S", ".", "-B", build_dir, f"-DCMAKE_BUILD_TYPE={build_config}"]
        vcpkg_toolchain_file = os.path.join(os.environ.get("VCPKG_ROOT", ""), "scripts/buildsystems/vcpkg.cmake")
        if os.path.exists(vcpkg_toolchain_file):
            cmake_configure_command.append(f"-DCMAKE_TOOLCHAIN_FILE={vcpkg_toolchain_file}")
        run_command(cmake_configure_command)
    else:
        print("Build directory already configured. Skipping CMake configuration.")

    # Build step
    if args.action in ["build", "test", "all", "bench"]:
        print(f"Building project (config: {build_config})...")
        run_command(["cmake", "--build", build_dir, "--config", build_config])
        print("Build complete.")

    # Test step
    if args.action in ["test", "all"]:
        print("Running C++ tests...")
        run_command(["ctest", "--output-on-failure", "--test-dir", build_dir])

        print("Running C# tests...")
        test_env = {}
        if sys.platform == "linux":
            native_lib_path = os.path.join(build_dir, "src")
            test_env["LD_LIBRARY_PATH"] = native_lib_path
            print(f"Setting LD_LIBRARY_PATH to: {native_lib_path}")

        run_command(
            ["dotnet", "test", os.path.join(project_root, "csharp/Yini.Tests/Yini.Tests.csproj"), "--configuration", build_config, "--no-build"],
            env=test_env
        )
        print("Tests complete.")

    # Benchmark step
    if args.action == "bench":
        print("--- Running Benchmarks ---")
        if build_config != "Release":
            print("Warning: Benchmarks should be run in Release mode for accurate results. Overriding configuration.")
            build_config = "Release"
            # We need to re-build if the config was changed
            print(f"Re-building project in Release mode...")
            run_command(["cmake", "--build", build_dir, "--config", build_config])
            print("Build complete.")

        print("Running C++ benchmarks...")
        cpp_bench_env = {}
        if sys.platform == "linux":
            native_lib_path = os.path.join(build_dir, "src")
            cpp_bench_env["LD_LIBRARY_PATH"] = native_lib_path
        run_command([os.path.join(build_dir, "tests/yini_bench")], env=cpp_bench_env)

        print("\nRunning C# benchmarks...")
        csharp_bench_env = {}
        if sys.platform == "linux":
            native_lib_path = os.path.join(build_dir, "src")
            csharp_bench_env["LD_LIBRARY_PATH"] = native_lib_path

        csharp_bench_project_path = os.path.join(project_root, "csharp/Yini.Tests")
        run_command(
            ["dotnet", "run", "--project", csharp_bench_project_path, "-c", "Release", "--", "--filter", "*BindingBenchmarks*"],
            env=csharp_bench_env
        )
        print("Benchmarks complete.")

    # Docs step
    if args.action in ["docs", "all"]:
        print("Generating documentation...")
        run_command(["cmake", "--build", build_dir, "--target", "doc"])
        print("Documentation generation complete.")

if __name__ == "__main__":
    main()