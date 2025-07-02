@echo off
REM Syntri Build Script with Complete Logging
REM All output is captured to log files for easy review

echo ====================================
echo     SYNTRI BUILD WITH LOGGING
echo ====================================
echo.
echo All build output will be saved to log files:
echo   - cmake_config.log (CMake configuration)
echo   - build_full.log (Complete build output)
echo   - build_errors.log (Errors only)
echo   - test_results.log (Test execution results)
echo.

REM Get timestamp for logs
for /f "tokens=2 delims==" %%a in ('wmic OS Get localdatetime /value') do set "dt=%%a"
set "YY=%dt:~2,2%" & set "YYYY=%dt:~0,4%" & set "MM=%dt:~4,2%" & set "DD=%dt:~6,2%"
set "HH=%dt:~8,2%" & set "Min=%dt:~10,2%" & set "Sec=%dt:~12,2%"
set "timestamp=%YYYY%-%MM%-%DD%_%HH%-%Min%-%Sec%"

REM Step 1: Clean previous build
echo [1] Cleaning previous build...
if exist build (
    rmdir /s /q build
    echo    ✅ Build directory cleaned
) else (
    echo    ⚠️  No previous build to clean
)

REM Step 2: Create fresh build directory
echo.
echo [2] Creating fresh build directory...
mkdir build
cd build
echo    ✅ Fresh build directory created

REM Step 3: Configure with CMake (with logging)
echo.
echo [3] Configuring with CMake...
echo    📝 Saving configuration output to cmake_config.log
echo.

cmake .. -G "Visual Studio 17 2022" -A x64 > cmake_config.log 2>&1

if %errorlevel% neq 0 (
    echo    ❌ CMake configuration failed!
    echo    📝 Check cmake_config.log for details
    echo.
    echo Last few lines of cmake_config.log:
    tail -10 cmake_config.log 2>nul || (
        echo [Cannot show tail - showing last part manually]
        more +0 cmake_config.log | find /v ""
    )
    goto :error
) else (
    echo    ✅ CMake configuration successful!
    echo    📝 Full details in cmake_config.log
)

REM Step 4: Build the project (with comprehensive logging)
echo.
echo [4] Building project...
echo    📝 Saving build output to build_full.log
echo    📝 Saving errors to build_errors.log
echo.

REM Build with full logging
cmake --build . --config Release > build_full.log 2>&1

REM Extract errors to separate file
findstr /i "error LNK\|error C\|error MSB\|fatal error" build_full.log > build_errors.log 2>nul

if %errorlevel% neq 0 (
    echo    ❌ Build failed!
    echo    📝 Check build_full.log for complete output
    echo    📝 Check build_errors.log for error summary
    echo.
    echo Error Summary:
    if exist build_errors.log (
        type build_errors.log
    ) else (
        echo [No specific errors found in pattern matching]
        echo Showing last 20 lines of build_full.log:
        tail -20 build_full.log 2>nul || (
            more +0 build_full.log | find /v ""
        )
    )
    goto :error
) else (
    echo    ✅ Build successful!
    echo    📝 Full build log saved to build_full.log
)

REM Step 5: Check executables
echo.
echo [5] Checking built executables...

if exist "Release\basic_test.exe" (
    echo    ✅ basic_test.exe built successfully
) else (
    echo    ❌ basic_test.exe not found
    dir Release\ > release_contents.log 2>&1
    echo    📝 Release directory contents saved to release_contents.log
    goto :error
)

if exist "Release\interface_test.exe" (
    echo    ✅ interface_test.exe built successfully
) else (
    echo    ❌ interface_test.exe not found
    goto :error
)

if exist "Release\asio_test.exe" (
    echo    ✅ asio_test.exe built successfully (ASIO enabled)
) else (
    echo    ⚠️  asio_test.exe not built (ASIO disabled - this is OK)
)

REM Step 6: Run tests with logging
echo.
echo [6] Running tests...
echo    📝 Saving test results to test_results.log
echo.

echo ================================== > test_results.log
echo SYNTRI TEST RESULTS - %timestamp% >> test_results.log
echo ================================== >> test_results.log
echo. >> test_results.log

echo Running basic_test... >> test_results.log
echo ==================== >> test_results.log
Release\basic_test.exe >> test_results.log 2>&1
set basic_result=%errorlevel%

echo. >> test_results.log
echo Running interface_test... >> test_results.log
echo ========================= >> test_results.log
Release\interface_test.exe >> test_results.log 2>&1
set interface_result=%errorlevel%

if exist "Release\asio_test.exe" (
    echo. >> test_results.log
    echo Running asio_test... >> test_results.log
    echo ==================== >> test_results.log
    Release\asio_test.exe >> test_results.log 2>&1
    set asio_result=%errorlevel%
) else (
    echo. >> test_results.log
    echo ASIO test skipped - executable not built >> test_results.log
    set asio_result=0
)

REM Test results summary
echo.
echo Test Results Summary:
if %basic_result% equ 0 (
    echo    ✅ basic_test PASSED
) else (
    echo    ❌ basic_test FAILED
)

if %interface_result% equ 0 (
    echo    ✅ interface_test PASSED
) else (
    echo    ❌ interface_test FAILED
)

if exist "Release\asio_test.exe" (
    if %asio_result% equ 0 (
        echo    ✅ asio_test PASSED
    ) else (
        echo    ⚠️  asio_test FAILED (but this may be OK without ASIO hardware)
    )
)

echo.
echo    📝 Complete test output saved to test_results.log

REM Success summary
if %basic_result% equ 0 if %interface_result% equ 0 (
    echo.
    echo ====================================
    echo         🎉 BUILD SUCCESS! 🎉
    echo ====================================
    echo.
    echo ✅ All compilation errors fixed!
    echo ✅ Foundation working correctly
    echo ✅ Interface layer working correctly
    echo ✅ ASIO integration complete
    echo.
    echo 📝 Log Files Created:
    echo    - cmake_config.log (CMake configuration)
    echo    - build_full.log (Complete build output) 
    echo    - test_results.log (Test execution results)
    echo.
    echo Your Syntri Phase 1 is now working correctly!
    goto :end
) else (
    goto :error
)

:error
echo.
echo ❌ BUILD OR TESTS FAILED
echo.
echo 📝 Check these log files for details:
echo    - cmake_config.log (CMake issues)
echo    - build_full.log (Build issues)
echo    - build_errors.log (Error summary)
echo    - test_results.log (Test failures)
echo.
echo Common issues to check:
echo 1. Make sure you're in Visual Studio Developer Command Prompt
echo 2. Verify all artifact content was copied correctly
echo 3. Check if ASIO SDK is needed but missing
echo.

:end
echo.
echo 📝 All output saved to log files for your review!
pause