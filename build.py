import subprocess
import argparse
import sys
import os

def run_command(command, cwd=None):
    """Runs a command and exits if it fails."""
    try:
        subprocess.run(command, check=True, shell=True, cwd=cwd)
    except subprocess.CalledProcessError as e:
        print(f"Error running command: {' '.join(command)}", file=sys.stderr)
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)

def main():
    parser = argparse.ArgumentParser(description="YINI build script")
    parser.add_argument("--configure", action="store_true", help="Run CMake configuration")
    parser.add_argument("--build", action="store_true", help="Build the project")
    parser.add_argument("--test", action="store_true", help="Run tests")
    parser.add_argument("--clean", action="store_true", help="Clean the build directory")
    args = parser.parse_args()

    build_dir = "build"

    if args.clean:
        if os.path.exists(build_dir):
            print("Cleaning build directory...")
            run_command(f"rm -rf {build_dir}")
        else:
            print("Build directory does not exist, nothing to clean.")
        return

    if not os.path.exists(build_dir):
        os.makedirs(build_dir)

    if args.configure or (not args.build and not args.test):
        print("Configuring CMake...")
        cmake_command = "cmake -S . -B build"
        run_command(cmake_command)

    if args.build or (not args.configure and not args.test):
        print("Building project...")
        build_command = "cmake --build build"
        run_command(build_command)

    if args.test:
        print("Running tests...")
        test_command = "ctest --output-on-failure"
        run_command(test_command, cwd=build_dir)

if __name__ == "__main__":
    main()