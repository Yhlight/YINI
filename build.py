import os
import subprocess
import sys

def run_command(command):
    """Runs a command and checks for errors."""
    try:
        subprocess.run(command, check=True, shell=True)
    except subprocess.CalledProcessError as e:
        print(f"Error executing command: {' '.join(command)}\n{e}", file=sys.stderr)
        sys.exit(1)

def main():
    """Builds the YINI project using CMake."""
    # Create build directory if it doesn't exist
    build_dir = "build"
    if not os.path.exists(build_dir):
        os.makedirs(build_dir)

    # Configure the project with CMake
    print("--- Configuring project with CMake ---")
    cmake_configure_command = f"cmake -S . -B {build_dir}"
    run_command(cmake_configure_command)

    # Build the project with CMake
    print(f"--- Building project in {build_dir} ---")
    cmake_build_command = f"cmake --build {build_dir}"
    run_command(cmake_build_command)

    print("--- Build successful ---")

if __name__ == "__main__":
    main()