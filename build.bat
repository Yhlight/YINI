@echo off
REM build.bat - A simple script to build the YINI project on Windows.
REM This script uses CMake to generate build files (e.g., for Visual Studio)
REM and then uses CMake's build tool mode to compile the project.

ECHO --- Starting YINI build process for Windows ---

REM --- Configuration ---
SET BUILD_DIR=build

REM --- Script ---

REM Create the build directory if it doesn't exist.
ECHO [1/4] Creating build directory: %BUILD_DIR%
if not exist %BUILD_DIR% mkdir %BUILD_DIR%

REM Navigate into the build directory.
ECHO [2/4] Changing directory to: %BUILD_DIR%
cd %BUILD_DIR%

REM Run CMake to configure the project and generate build files.
REM CMake will automatically detect the installed version of Visual Studio.
REM For other compilers, you might need to specify a generator with -G "Generator Name"
ECHO [3/4] Running CMake to configure the project...
cmake ..

REM Run CMake's build tool to compile the project.
REM This is a generic command that works for any generator (e.g., Visual Studio, MinGW).
ECHO [4/4] Compiling the project...
cmake --build .

ECHO --- Build complete! ---
ECHO The executable can usually be found at: %BUILD_DIR%\Debug\yini.exe or %BUILD_DIR%\Release\yini.exe
