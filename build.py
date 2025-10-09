import os
import subprocess
import sys
import shutil

def main():
    """
    Automates the CMake build process for the YINI project.
    """
    build_dir = "build"
    script_dir = os.path.dirname(os.path.abspath(__file__))
    build_path = os.path.join(script_dir, build_dir)

    # 1. Create build directory
    if not os.path.exists(build_path):
        print(f"--- Creating build directory at: {build_path} ---")
        os.makedirs(build_path)

    # 2. Configure project with CMake
    print("--- Configuring project ---")
    cmake_args = ["cmake", "-S", script_dir, "-B", build_path]
    result = subprocess.run(cmake_args, check=False)
    if result.returncode != 0:
        print("CMake configuration failed.", file=sys.stderr)
        sys.exit(1)

    # 3. Build project
    print("--- Building project ---")
    build_args = ["cmake", "--build", build_path]
    result = subprocess.run(build_args, check=False)
    if result.returncode != 0:
        print("Build failed.", file=sys.stderr)
        sys.exit(1)

    print("\nBuild completed successfully!")

if __name__ == "__main__":
    main()