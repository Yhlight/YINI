import os
import subprocess
import sys
import argparse

def run_command(args, cwd):
    """Runs a command and prints its output in real-time."""
    print(f"Running command: {' '.join(args)} in {cwd}")
    process = subprocess.Popen(
        args,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        cwd=cwd,
        bufsize=1,
        universal_newlines=True,
    )
    for line in process.stdout:
        print(line, end="")
    process.wait()
    if process.returncode != 0:
        print(f"Error: Command exited with code {process.returncode}")
        sys.exit(process.returncode)

def main():
    """Main build script."""
    parser = argparse.ArgumentParser(description="YINI build script.")
    parser.add_argument(
        "--test", action="store_true", help="Run tests after building."
    )
    args = parser.parse_args()

    build_dir = "build"
    os.makedirs(build_dir, exist_ok=True)

    # Configure CMake
    cmake_args = ["cmake", ".."]
    run_command(cmake_args, build_dir)

    # Build the project
    build_args = ["cmake", "--build", "."]
    run_command(build_args, build_dir)

    # Run tests if requested
    if args.test:
        test_args = ["ctest", "--output-on-failure"]
        run_command(test_args, build_dir)

if __name__ == "__main__":
    main()