import os
import subprocess
import sys

def run_command(command):
    """Executes a command and exits if it fails."""
    try:
        subprocess.run(command, check=True, shell=True)
    except subprocess.CalledProcessError as e:
        print(f"Error executing command: {' '.join(command)}\n{e}", file=sys.stderr)
        sys.exit(1)

def main():
    """Configures and builds the project using CMake."""
    # Create a build directory if it doesn't exist
    build_dir = "build"
    if not os.path.exists(build_dir):
        os.makedirs(build_dir)

    # Configure the project with CMake
    print("--- Configuring project ---")
    run_command(f"cmake -S . -B {build_dir}")

    # Build the project
    print("--- Building project ---")
    run_command(f"cmake --build {build_dir}")

if __name__ == "__main__":
    main()