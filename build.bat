@echo off
REM build.bat - A script to build the YINI C++ library and the C# example on Windows.

ECHO --- [1/4] Building C++ Shared Library ---

REM --- Configuration ---
SET BUILD_DIR=build
SET CSHARP_DIR=csharp_example

REM --- C++ Build ---
REM Create the build directory if it doesn't exist.
if not exist %BUILD_DIR% mkdir %BUILD_DIR%

REM Navigate into the build directory.
cd %BUILD_DIR%

REM Run CMake to configure the project.
cmake ..

REM Run CMake's build tool to compile the project.
cmake --build . --config Release
cd ..
ECHO --- C++ Library build complete! ---


ECHO.
ECHO --- [2/4] Preparing C# Example ---
REM Copy the native library to the C# project directory so it can be found at runtime.
REM We copy from the Release folder, which is the default for `cmake --build .`
ECHO Copying yini.dll to %CSHARP_DIR%...
copy "%BUILD_DIR%\Release\yini.dll" "%CSHARP_DIR%\"

ECHO.
ECHO --- [3/4] Building C# Project ---
cd %CSHARP_DIR%
dotnet build

ECHO.
ECHO --- [4/4] Running C# Example ---
dotnet run

cd ..
ECHO.
ECHO --- Full build and test complete! ---
