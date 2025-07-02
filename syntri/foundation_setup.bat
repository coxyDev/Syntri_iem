@echo off
REM Syntri Foundation Setup - Phase 1 Clean Start
REM This script helps you create the clean project structure

echo ================================================================
echo          SYNTRI PHASE 1 - FOUNDATION SETUP
echo ================================================================
echo.
echo This script will help you create a clean Syntri project
echo from scratch, avoiding all the complexity issues from before.
echo.
echo Goal: Get a simple foundation test compiling and running
echo.
pause

REM Step 1: Verify prerequisites
echo [1] Verifying prerequisites...
echo.

cmake --version >nul 2>&1
if %errorlevel% neq 0 (
    echo ❌ CMake not found! Please install CMake first.
    pause
    exit /b 1
) else (
    echo ✅ CMake found
)

where cl >nul 2>&1
if %errorlevel% neq 0 (
    echo ❌ Visual Studio compiler not found! 
    echo Please run this from a Visual Studio Developer Command Prompt.
    pause
    exit /b 1
) else (
    echo ✅ Visual Studio compiler found
)

echo.

REM Step 2: Create project structure
echo [2] Creating clean project structure...
echo.

if exist syntri (
    echo ❌ 'syntri' directory already exists!
    echo Please delete it first for a completely clean start.
    pause
    exit /b 1
)

echo Creating directories...
mkdir syntri
mkdir syntri\include
mkdir syntri\include\syntri
mkdir syntri\src
mkdir syntri\src\core
mkdir syntri\src\hardware
mkdir syntri\test
mkdir syntri\external

echo ✅ Project structure created
echo.

REM Step 3: Copy your existing files
echo [3] Setting up existing files...
echo.
echo ACTION REQUIRED: Please copy your existing files to syntri\:
echo.
echo ✅ Copy your README.md to syntri\README.md
echo ✅ Copy your LICENSE to syntri\LICENSE  
echo ✅ Copy your .gitignore to syntri\.gitignore
echo.
echo Press any key when you've copied these files...
pause

REM Step 4: Create foundation files
echo [4] Creating foundation files...
echo.
echo ACTION REQUIRED: Create these files in your syntri directory:
echo.
echo 1. Copy 'starter_cmake' artifact content to: syntri\CMakeLists.txt
echo 2. Copy 'basic_types' artifact content to: syntri\include\syntri\types.h
echo 3. Copy 'basic_test' artifact content to: syntri\test\basic_test.cpp
echo.
echo These are the minimal foundation files that should compile cleanly.
echo.
echo Press any key when you've created all three files...
pause

echo.
echo [5] Verifying files exist...
cd syntri

if exist "CMakeLists.txt" (
    echo ✅ CMakeLists.txt found
) else (
    echo ❌ CMakeLists.txt missing
    goto :error
)

if exist "include\syntri\types.h" (
    echo ✅ types.h found
) else (
    echo ❌ types.h missing
    goto :error
)

if exist "test\basic_test.cpp" (
    echo ✅ basic_test.cpp found
) else (
    echo ❌ basic_test.cpp missing
    goto :error
)

echo.

REM Step 6: Test compilation
echo [6] Testing foundation compilation...
echo.

if exist build rmdir /s /q build
mkdir build
cd build

echo Configuring with CMake...
cmake .. -G "Visual Studio 17 2022" -A x64 > cmake_log.txt 2>&1
if %errorlevel% neq 0 (
    echo ❌ CMake configuration failed!
    echo.
    echo Error details:
    type cmake_log.txt
    goto :error
) else (
    echo ✅ CMake configuration successful!
)

echo.
echo Building foundation test...
cmake --build . --config Release > build_log.txt 2>&1
if %errorlevel% neq 0 (
    echo ❌ Foundation build failed!
    echo.
    echo Error details:
    type build_log.txt
    goto :error
) else (
    echo ✅ Foundation build successful!
)

echo.

REM Step 7: Run foundation test
echo [7] Running foundation test...
echo.

if exist "Release\basic_test.exe" (
    echo Running foundation test...
    Release\basic_test.exe
    echo.
    if %errorlevel% equ 0 (
        echo ✅ Foundation test PASSED!
    ) else (
        echo ❌ Foundation test failed
        goto :error
    )
) else (
    echo ❌ basic_test.exe not found
    dir Release\
    goto :error
)

echo.
echo ================================================================
echo                    🎉 SUCCESS! 🎉
echo ================================================================
echo.
echo Your Syntri foundation is working perfectly!
echo.
echo What you've achieved:
echo ✅ Clean project structure
echo ✅ Basic compilation working
echo ✅ Audio types defined
echo ✅ Hardware enums working
echo ✅ Foundation test passing
echo.
echo Next Phase 1 steps:
echo 1. Add audio interface abstraction
echo 2. Add ASIO integration
echo 3. Add hardware detection
echo 4. Add basic audio I/O
echo 5. Add latency measurement
echo.
echo Ready to proceed with Phase 1 development!
echo.
goto :end

:error
echo.
echo ❌ FOUNDATION SETUP FAILED
echo.
echo Please check the error messages above and try again.
echo Make sure you've copied all the artifact contents exactly.
echo.

:end
pause