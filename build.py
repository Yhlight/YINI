import os
import subprocess
import sys

def run_command(command):
    """Runs a command and checks for errors."""
    try:
        subprocess.run(command, check=True, shell=True)
    except subprocess.CalledProcessError as e:
        print(f"Error executing command: {command}", file=sys.stderr)
        sys.exit(1)

def main():
    """Main function to build the project."""
    # Create build directory if it doesn't exist
    if not os.path.exists("build"):
        os.makedirs("build")

    # Change to the build directory
    os.chdir("build")

    # Run CMake and build the project
    run_command("cmake ..")
    run_command("cmake --build .")

    print("Build successful!")

if __name__ == "__main__":
    main()