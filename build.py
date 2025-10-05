import os
import subprocess
import sys
import platform

def run_command(command, cwd=None):
    """Runs a command and checks for errors."""
    try:
        subprocess.run(command, check=True, shell=True, cwd=cwd)
    except subprocess.CalledProcessError as e:
        print(f"Error executing command: {command}", file=sys.stderr)
        sys.exit(1)

def main():
    """Main function to build the project."""
    project_root = os.path.abspath(os.path.dirname(__file__))
    build_dir = os.path.join(project_root, "build")

    # Create build directory if it doesn't exist
    if not os.path.exists(build_dir):
        os.makedirs(build_dir)

    # Run CMake and build the C++ project
    print("--- Building C++ Project ---")
    run_command("cmake ..", cwd=build_dir)
    run_command("cmake --build .", cwd=build_dir)
    print("--- C++ Build Successful ---")

    # Build the C# project
    print("\n--- Building C# Project ---")
    csharp_dir = os.path.join(project_root, "csharp")

    native_lib_path = ""
    system = platform.system()
    if system == "Linux":
        native_lib_path = os.path.join(build_dir, "src", "libYini.so")
    elif system == "Windows":
        # Assuming Release config for Windows build
        native_lib_path = os.path.join(build_dir, "src", "Release", "Yini.dll")
    elif system == "Darwin": # macOS
        native_lib_path = os.path.join(build_dir, "src", "libYini.dylib")

    if not os.path.exists(native_lib_path):
        print(f"Error: Native library not found at {native_lib_path}", file=sys.stderr)
        sys.exit(1)

    # Pass the native library path as a property to the C# build
    dotnet_command = f'dotnet build -p:NativeLibPath="{native_lib_path}"'
    run_command(dotnet_command, cwd=csharp_dir)
    print("--- C# Build Successful ---")

    print("\nBuild successful!")

if __name__ == "__main__":
    main()