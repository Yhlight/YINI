import os
import subprocess
import sys

def run_command(command):
    """Runs a command and checks for errors."""
    try:
        subprocess.run(command, check=True, shell=True)
    except subprocess.CalledProcessError as e:
        print(f"Error executing command: {e}", file=sys.stderr)
        sys.exit(1)

def main():
    """Builds the YINI project using CMake."""
    if not os.path.exists("build"):
        os.makedirs("build")

    os.chdir("build")

    run_command("cmake ..")
    run_command("cmake --build .")

    os.chdir("..")

if __name__ == "__main__":
    main()