#!/usr/bin/env python3
"""
YINI Build Script
Automates CMake configuration and build process
"""

import os
import sys
import subprocess
import argparse
import shutil
from pathlib import Path

class YINIBuilder:
    def __init__(self, workspace_dir=None):
        self.workspace_dir = Path(workspace_dir or os.getcwd())
        self.build_dir = self.workspace_dir / "build"
        self.src_dir = self.workspace_dir
        
    def clean(self):
        """Clean build directory"""
        if self.build_dir.exists():
            print(f"Cleaning build directory: {self.build_dir}")
            shutil.rmtree(self.build_dir)
        else:
            print("Build directory does not exist, nothing to clean")
    
    def configure(self, build_type="Debug"):
        """Configure CMake project"""
        self.build_dir.mkdir(parents=True, exist_ok=True)
        
        cmd = [
            "cmake",
            "-S", str(self.src_dir),
            "-B", str(self.build_dir),
            f"-DCMAKE_BUILD_TYPE={build_type}"
        ]
        
        print(f"Configuring CMake project...")
        print(f"Command: {' '.join(cmd)}")
        
        result = subprocess.run(cmd, cwd=self.workspace_dir)
        if result.returncode != 0:
            print("CMake configuration failed!")
            sys.exit(1)
        
        print("CMake configuration successful!")
    
    def build(self, target=None):
        """Build the project"""
        if not self.build_dir.exists():
            print("Build directory does not exist. Running configure first...")
            self.configure()
        
        cmd = ["cmake", "--build", str(self.build_dir)]
        if target:
            cmd.extend(["--target", target])
        
        print(f"Building project...")
        print(f"Command: {' '.join(cmd)}")
        
        result = subprocess.run(cmd, cwd=self.workspace_dir)
        if result.returncode != 0:
            print("Build failed!")
            sys.exit(1)
        
        print("Build successful!")
    
    def test(self):
        """Run tests"""
        if not self.build_dir.exists():
            print("Build directory does not exist. Please build first.")
            sys.exit(1)
        
        cmd = ["ctest", "--output-on-failure"]
        
        print(f"Running tests...")
        print(f"Command: {' '.join(cmd)}")
        
        result = subprocess.run(cmd, cwd=self.build_dir)
        if result.returncode != 0:
            print("Tests failed!")
            sys.exit(1)
        
        print("All tests passed!")
    
    def install(self):
        """Install the project"""
        cmd = ["cmake", "--install", str(self.build_dir)]
        
        print(f"Installing project...")
        result = subprocess.run(cmd, cwd=self.workspace_dir)
        if result.returncode != 0:
            print("Installation failed!")
            sys.exit(1)
        
        print("Installation successful!")

def main():
    parser = argparse.ArgumentParser(description="YINI Build Script")
    parser.add_argument("action", choices=["clean", "configure", "build", "test", "install", "all"],
                       help="Action to perform")
    parser.add_argument("--build-type", default="Debug", choices=["Debug", "Release"],
                       help="CMake build type")
    parser.add_argument("--target", help="Specific build target")
    
    args = parser.parse_args()
    
    builder = YINIBuilder()
    
    if args.action == "clean":
        builder.clean()
    elif args.action == "configure":
        builder.configure(args.build_type)
    elif args.action == "build":
        builder.build(args.target)
    elif args.action == "test":
        builder.test()
    elif args.action == "install":
        builder.install()
    elif args.action == "all":
        builder.clean()
        builder.configure(args.build_type)
        builder.build()
        builder.test()
    
    return 0

if __name__ == "__main__":
    sys.exit(main())
