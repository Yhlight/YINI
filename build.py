import os
import subprocess
import sys
import platform
import argparse

def run_command(command, cwd=None, env=None):
    """Runs a command, inheriting the environment and optionally adding to it."""
    try:
        full_env = os.environ.copy()
        if env:
            full_env.update(env)

        subprocess.run(command, check=True, shell=True, cwd=cwd, env=full_env)
        print(f"Successfully executed: {command}")
    except subprocess.CalledProcessError as e:
        print(f"Error executing command: {command}", file=sys.stderr)
        print(f"Return code: {e.returncode}", file=sys.stderr)
        if e.stdout:
            print(f"stdout:\n{e.stdout}", file=sys.stderr)
        if e.stderr:
            print(f"stderr:\n{e.stderr}", file=sys.stderr)
        sys.exit(1)

def get_native_lib_path(build_dir):
    """Determines the path to the built native library."""
    system = platform.system()
    if system == "Linux":
        return os.path.join(build_dir, "src", "libYini.so")
    elif system == "Windows":
        # CMake default for single-config generators on Windows is Debug
        # For multi-config (like VS), we must specify. Let's assume Release.
        return os.path.join(build_dir, "src", "Release", "Yini.dll")
    elif system == "Darwin": # macOS
        return os.path.join(build_dir, "src", "libYini.dylib")
    return ""

def do_build(project_root, build_dir, csharp_dir):
    """Builds the C++ and C# projects."""
    if not os.path.exists(build_dir):
        os.makedirs(build_dir)

    print("\n--- Building C++ Project ---")
    run_command("cmake ..", cwd=build_dir)
    # For multi-config generators like Visual Studio, specify the config here.
    run_command("cmake --build . --config Release", cwd=build_dir)
    print("--- C++ Build Successful ---")

    print("\n--- Building C# Project ---")
    native_lib_path = get_native_lib_path(build_dir)
    if not os.path.exists(native_lib_path):
        print(f"Error: Native library not found at {native_lib_path}", file=sys.stderr)
        sys.exit(1)

    dotnet_command = f'dotnet build -p:NativeLibPath="{native_lib_path}"'
    run_command(dotnet_command, cwd=csharp_dir)
    print("--- C# Build Successful ---")

def do_test(project_root, build_dir, csharp_dir):
    """Builds and tests the C++ and C# projects."""
    do_build(project_root, build_dir, csharp_dir)

    print("\n--- Running C++ Tests ---")
    run_command("ctest --output-on-failure --test-dir .", cwd=build_dir)
    print("--- C++ Tests Passed ---")

    print("\n--- Running C# Tests ---")
    test_env = None
    if platform.system() == "Linux":
        # For Linux, dotnet test needs to be able to find the native .so file.
        lib_dir = os.path.dirname(get_native_lib_path(build_dir))
        test_env = {"LD_LIBRARY_PATH": lib_dir}
        print(f"Temporarily setting LD_LIBRARY_PATH to: {lib_dir}")

    run_command("dotnet test", cwd=csharp_dir, env=test_env)
    print("--- C# Tests Passed ---")

def main():
    """Main function to parse commands and execute them."""
    parser = argparse.ArgumentParser(description="Build and test script for the YINI project.")
    # Keep it simple, just one command for now.
    parser.add_argument('command', nargs='?', default='build', choices=['build', 'test'],
                        help="The command to execute: 'build' (default) or 'test'.")
    args = parser.parse_args()

    project_root = os.path.abspath(os.path.dirname(__file__))
    build_dir = os.path.join(project_root, "build")
    csharp_dir = os.path.join(project_root, "csharp")

    if args.command == "build":
        do_build(project_root, build_dir, csharp_dir)
        print("\nBuild successful!")
    elif args.command == "test":
        do_test(project_root, build_dir, csharp_dir)
        print("\nAll tests passed successfully!")

if __name__ == "__main__":
    main()