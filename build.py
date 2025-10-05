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
    parser.add_argument("action", choices=["build", "test", "docs", "clean", "all"], help="Action to perform")
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

    # Configure step for all actions except 'clean'
    if not os.path.exists(os.path.join(build_dir, "CMakeCache.txt")):
        print("Configuring CMake...")
        os.makedirs(build_dir, exist_ok=True)
        cmake_configure_command = ["cmake", "-S", ".", "-B", build_dir, f"-DCMAKE_BUILD_TYPE={args.config}"]
        # Check for vcpkg
        vcpkg_toolchain_file = os.path.join(os.environ.get("VCPKG_ROOT", ""), "scripts/buildsystems/vcpkg.cmake")
        if os.path.exists(vcpkg_toolchain_file):
            cmake_configure_command.append(f"-DCMAKE_TOOLCHAIN_FILE={vcpkg_toolchain_file}")
        run_command(cmake_configure_command)
    else:
        print("Build directory already configured. Skipping CMake configuration.")

    # Build step
    if args.action in ["build", "test", "all"]:
        print(f"Building project (config: {args.config})...")
        run_command(["cmake", "--build", build_dir, "--config", args.config])
        print("Build complete.")

    # Test step
    if args.action in ["test", "all"]:
        print("Running C++ tests...")
        run_command(["ctest", "--output-on-failure", "--test-dir", build_dir])

        print("Running C# tests...")
        # Set LD_LIBRARY_PATH for Linux to find the native library
        test_env = {}
        if sys.platform == "linux":
            native_lib_path = os.path.join(build_dir, "src")
            test_env["LD_LIBRARY_PATH"] = native_lib_path
            print(f"Setting LD_LIBRARY_PATH to: {native_lib_path}")

        run_command(
            ["dotnet", "test", os.path.join(project_root, "csharp/YiniSolution.sln"), "--configuration", args.config, "--no-build"],
            env=test_env
        )
        print("Tests complete.")

    # Docs step
    if args.action in ["docs", "all"]:
        print("Generating documentation...")
        run_command(["cmake", "--build", build_dir, "--target", "doc"])
        print("Documentation generation complete.")

if __name__ == "__main__":
    main()