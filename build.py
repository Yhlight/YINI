#!/usr/bin/env python3
"""
YINI Build Script
This script helps build the YINI project using CMake
"""

import os
import sys
import subprocess
import argparse
import shutil
from pathlib import Path


def get_project_root():
    """Get the project root directory"""
    return Path(__file__).parent.absolute()


def run_command(cmd, cwd=None):
    """Run a shell command and handle errors"""
    print(f"Running: {' '.join(cmd)}")
    result = subprocess.run(cmd, cwd=cwd, capture_output=True, text=True)
    
    if result.stdout:
        print(result.stdout)
    if result.stderr:
        print(result.stderr, file=sys.stderr)
    
    if result.returncode != 0:
        print(f"Command failed with exit code {result.returncode}")
        sys.exit(1)
    
    return result


def clean_build(build_dir):
    """Clean the build directory"""
    if build_dir.exists():
        print(f"Cleaning build directory: {build_dir}")
        shutil.rmtree(build_dir)
    build_dir.mkdir(parents=True, exist_ok=True)


def configure_cmake(build_dir, project_root, build_type="Debug", options=None):
    """Configure CMake"""
    cmd = [
        "cmake",
        f"-DCMAKE_BUILD_TYPE={build_type}",
        "-S", str(project_root),
        "-B", str(build_dir)
    ]
    
    if options:
        cmd.extend(options)
    
    run_command(cmd)


def build_project(build_dir, target=None, jobs=None):
    """Build the project"""
    cmd = ["cmake", "--build", str(build_dir)]
    
    if target:
        cmd.extend(["--target", target])
    
    if jobs:
        cmd.extend(["--parallel", str(jobs)])
    
    run_command(cmd)


def run_tests(build_dir):
    """Run tests"""
    cmd = ["ctest", "--output-on-failure", "--test-dir", str(build_dir)]
    run_command(cmd)


def install_project(build_dir, install_prefix=None):
    """Install the project"""
    cmd = ["cmake", "--install", str(build_dir)]
    
    if install_prefix:
        cmd.extend(["--prefix", str(install_prefix)])
    
    run_command(cmd)


def main():
    parser = argparse.ArgumentParser(description="Build YINI project")
    parser.add_argument(
        "--clean",
        action="store_true",
        help="Clean build directory before building"
    )
    parser.add_argument(
        "--build-type",
        choices=["Debug", "Release", "RelWithDebInfo", "MinSizeRel"],
        default="Debug",
        help="CMake build type (default: Debug)"
    )
    parser.add_argument(
        "--build-dir",
        type=Path,
        default=None,
        help="Build directory (default: <project_root>/build)"
    )
    parser.add_argument(
        "--target",
        help="Specific target to build"
    )
    parser.add_argument(
        "--jobs", "-j",
        type=int,
        help="Number of parallel build jobs"
    )
    parser.add_argument(
        "--test",
        action="store_true",
        help="Run tests after building"
    )
    parser.add_argument(
        "--install",
        action="store_true",
        help="Install the project after building"
    )
    parser.add_argument(
        "--install-prefix",
        type=Path,
        help="Installation prefix"
    )
    parser.add_argument(
        "--configure-only",
        action="store_true",
        help="Only configure, don't build"
    )
    
    args = parser.parse_args()
    
    # Get project root and build directory
    project_root = get_project_root()
    build_dir = args.build_dir if args.build_dir else project_root / "build"
    
    print(f"Project root: {project_root}")
    print(f"Build directory: {build_dir}")
    print(f"Build type: {args.build_type}")
    
    # Clean if requested
    if args.clean:
        clean_build(build_dir)
    else:
        build_dir.mkdir(parents=True, exist_ok=True)
    
    # Configure
    configure_cmake(build_dir, project_root, args.build_type)
    
    # Build if not configure-only
    if not args.configure_only:
        build_project(build_dir, args.target, args.jobs)
    
    # Run tests if requested
    if args.test:
        run_tests(build_dir)
    
    # Install if requested
    if args.install:
        install_project(build_dir, args.install_prefix)
    
    print("Build completed successfully!")


if __name__ == "__main__":
    main()
