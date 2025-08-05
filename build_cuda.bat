@echo off
echo Building MiniRenderer with CUDA support...

REM Check if CUDA is available
nvcc --version >nul 2>&1
if %errorlevel% neq 0 (
    echo WARNING: CUDA toolkit not found. Building without CUDA support.
    echo Make sure CUDA toolkit is installed and nvcc is in your PATH.
)

REM Create build directory
if not exist build mkdir build
cd build

echo Running CMake configuration...
cmake .. -DCMAKE_BUILD_TYPE=Release

if %errorlevel% neq 0 (
    echo CMake configuration failed!
    pause
    exit /b 1
)

echo Building project...
cmake --build . --config Release

if %errorlevel% neq 0 (
    echo Build failed!
    pause
    exit /b 1
)

echo Build completed successfully!
echo.
echo To run the renderer:
echo   cd Debug (or Release)
echo   MiniRenderer.exe
echo.
pause
