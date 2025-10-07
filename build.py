import argparse
import os
import shutil
import subprocess
import sys

def run_command(command, cwd=None):
    """Runs a command and checks for errors."""
    print(f"Running command: {' '.join(command)}")
    result = subprocess.run(command, cwd=cwd)
    if result.returncode != 0:
        print(f"Error running command: {' '.join(command)}")
        sys.exit(1)

def main():
    parser = argparse.ArgumentParser(description="YINI build script")
    parser.add_argument("--clean", action="store_true", help="Clean the build directory.")
    parser.add_argument("--build", action="store_true", default=True, help="Build the project.")
    parser.add_argument("--test", action="store_true", help="Run tests after building.")
    args = parser.parse_args()

    build_dir = "build"

    if args.clean:
        print("Cleaning the build directory...")
        if os.path.exists(build_dir):
            shutil.rmtree(build_dir)
        print("Clean complete.")
        # if only clean is requested, don't do anything else
        if not (args.build or args.test):
            return


    if not os.path.exists(build_dir):
        os.makedirs(build_dir)

    # Configure
    run_command(["cmake", ".."], cwd=build_dir)

    # Build
    if args.build or args.test:
        run_command(["cmake", "--build", "."], cwd=build_dir)

    # Test
    if args.test:
        run_command(["ctest", "--output-on-failure"], cwd=build_dir)


if __name__ == "__main__":
    main()