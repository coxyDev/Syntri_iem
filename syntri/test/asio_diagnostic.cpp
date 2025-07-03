// test/asio_diagnostic.cpp
// ASIO Diagnostic Tool - Tests ASIO SDK availability and basic functionality
// This helps us verify the ASIO environment before integration

#include <iostream>
#include <string>

// Test ASIO SDK compilation
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <combaseapi.h>

// Try to include ASIO headers
#ifdef ASIO_SDK_AVAILABLE
#include "asiosys.h"
#include "asio.h"
#include "asiodrivers.h"
#include "asiolist.h"

// Test ASIO function availability
bool testASIOFunctions() {
    std::cout << "Testing ASIO function availability..." << std::endl;

    // These should be available if ASIO SDK is properly included
    std::cout << "  ASIOInit function: " << (ASIOInit ? "Available" : "Missing") << std::endl;
    std::cout << "  ASIOExit function: " << (ASIOExit ? "Available" : "Missing") << std::endl;
    std::cout << "  ASIOGetChannels function: " << (ASIOGetChannels ? "Available" : "Missing") << std::endl;
    std::cout << "  ASIOStart function: " << (ASIOStart ? "Available" : "Missing") << std::endl;
    std::cout << "  ASIOStop function: " << (ASIOStop ? "Available" : "Missing") << std::endl;

    return (ASIOInit && ASIOExit && ASIOGetChannels && ASIOStart && ASIOStop);
}

bool testASIODrivers() {
    std::cout << "Testing ASIO driver enumeration..." << std::endl;

    try {
        AsioDrivers* drivers = new AsioDrivers();
        if (!drivers) {
            std::cout << "  Failed to create AsioDrivers object" << std::endl;
            return false;
        }

        char** driver_names = new char* [32];
        for (int i = 0; i < 32; i++) {
            driver_names[i] = new char[32];
        }

        long driver_count = drivers->getDriverNames(driver_names, 32);
        std::cout << "  Found " << driver_count << " ASIO driver(s):" << std::endl;

        for (long i = 0; i < driver_count; i++) {
            std::cout << "    " << (i + 1) << ". " << driver_names[i] << std::endl;
        }

        // Cleanup
        for (int i = 0; i < 32; i++) {
            delete[] driver_names[i];
        }
        delete[] driver_names;
        delete drivers;

        return driver_count > 0;

    }
    catch (const std::exception& e) {
        std::cout << "  Exception during driver enumeration: " << e.what() << std::endl;
        return false;
    }
    catch (...) {
        std::cout << "  Unknown exception during driver enumeration" << std::endl;
        return false;
    }
}

#endif // ASIO_SDK_AVAILABLE
#endif // _WIN32

int main() {
    std::cout << "=====================================" << std::endl;
    std::cout << "    ASIO DIAGNOSTIC TOOL" << std::endl;
    std::cout << "=====================================" << std::endl;
    std::cout << std::endl;

    // Test 1: Platform Check
    std::cout << "Test 1: Platform Compatibility" << std::endl;
#ifdef _WIN32
    std::cout << "  Platform: Windows (ASIO supported)" << std::endl;
#else
    std::cout << "  Platform: Non-Windows (ASIO not supported)" << std::endl;
    std::cout << "  ASIO is Windows-only. Use other audio APIs on this platform." << std::endl;
    return 0;
#endif
    std::cout << std::endl;

    // Test 2: ASIO SDK Compilation
    std::cout << "Test 2: ASIO SDK Compilation" << std::endl;
#ifdef ASIO_SDK_AVAILABLE
    std::cout << "  ASIO SDK: Compiled in successfully" << std::endl;
    std::cout << "  Headers: asiosys.h, asio.h, asiodrivers.h available" << std::endl;
#else
    std::cout << "  ASIO SDK: NOT compiled in" << std::endl;
    std::cout << "  Reason: ASIO_SDK_AVAILABLE not defined" << std::endl;
    std::cout << std::endl;
    std::cout << "To enable ASIO support:" << std::endl;
    std::cout << "  1. Download ASIO SDK 2.3.3 from Steinberg" << std::endl;
    std::cout << "  2. Extract to C:\\asiosdk_2.3.3\\" << std::endl;
    std::cout << "  3. Rebuild with cmake .. -DENABLE_ASIO_SUPPORT=ON" << std::endl;
    return 1;
#endif
    std::cout << std::endl;

#ifdef ASIO_SDK_AVAILABLE
    // Test 3: COM Initialization
    std::cout << "Test 3: Windows COM System" << std::endl;
    HRESULT hr = CoInitialize(nullptr);
    if (SUCCEEDED(hr) || hr == RPC_E_CHANGED_MODE) {
        std::cout << "  COM: Initialized successfully" << std::endl;
    }
    else {
        std::cout << "  COM: Initialization failed (HRESULT: " << hr << ")" << std::endl;
        return 1;
    }
    std::cout << std::endl;

    // Test 4: ASIO Function Availability
    std::cout << "Test 4: ASIO Function Availability" << std::endl;
    if (testASIOFunctions()) {
        std::cout << "  All core ASIO functions are available" << std::endl;
    }
    else {
        std::cout << "  Some ASIO functions are missing" << std::endl;
        CoUninitialize();
        return 1;
    }
    std::cout << std::endl;

    // Test 5: ASIO Driver Detection
    std::cout << "Test 5: ASIO Driver Detection" << std::endl;
    bool drivers_found = testASIODrivers();
    if (drivers_found) {
        std::cout << "  ASIO drivers detected successfully" << std::endl;
    }
    else {
        std::cout << "  No ASIO drivers found" << std::endl;
        std::cout << "  This is normal if no ASIO hardware is connected" << std::endl;
    }
    std::cout << std::endl;

    // Cleanup
    CoUninitialize();

    // Results Summary
    std::cout << "=====================================" << std::endl;
    std::cout << "         DIAGNOSTIC RESULTS" << std::endl;
    std::cout << "=====================================" << std::endl;
    std::cout << "Platform: Windows (OK)" << std::endl;
    std::cout << "ASIO SDK: Compiled in (OK)" << std::endl;
    std::cout << "COM System: Working (OK)" << std::endl;
    std::cout << "ASIO Functions: Available (OK)" << std::endl;
    std::cout << "ASIO Drivers: " << (drivers_found ? "Found" : "None") << std::endl;
    std::cout << std::endl;

    if (drivers_found) {
        std::cout << "STATUS: Ready for ASIO integration!" << std::endl;
        std::cout << "Next step: Implement ASIO audio interface" << std::endl;
    }
    else {
        std::cout << "STATUS: ASIO system working, no drivers detected" << std::endl;
        std::cout << "Next step: Connect ASIO hardware or proceed with simulation" << std::endl;
    }

    std::cout << "=====================================" << std::endl;
#endif // ASIO_SDK_AVAILABLE

    return 0;
}