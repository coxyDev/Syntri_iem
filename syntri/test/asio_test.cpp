// test/asio_test.cpp
// ASIO integration testing that works with your existing structure
#include "syntri/asio_interface.h"
#include "syntri/audio_interface.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <memory>

using namespace Syntri;

// Test configuration
const int TEST_SAMPLE_RATE = SAMPLE_RATE_48K;
const int TEST_BUFFER_SIZE = BUFFER_SIZE_LOW;
const int TEST_DURATION_MS = 1000;  // 1 second

// Test results tracking
struct TestResult {
    bool passed;
    std::string description;
    std::string details;
};

std::vector<TestResult> test_results;

void addTestResult(bool passed, const std::string& description, const std::string& details = "") {
    test_results.push_back({ passed, description, details });
    std::cout << (passed ? "PASS " : "FAIL ") << description;
    if (!details.empty()) {
        std::cout << " - " << details;
    }
    std::cout << std::endl;
}

void printTestSummary() {
    int passed = 0;
    int total = test_results.size();

    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "ASIO Test Summary" << std::endl;
    std::cout << std::string(50, '=') << std::endl;

    for (const auto& result : test_results) {
        if (result.passed) passed++;
        std::cout << (result.passed ? "PASS" : "FAIL") << " " << result.description << std::endl;
        if (!result.details.empty()) {
            std::cout << "     " << result.details << std::endl;
        }
    }

    std::cout << std::string(50, '=') << std::endl;
    std::cout << "Results: " << passed << "/" << total << " tests passed" << std::endl;

    if (passed == total) {
        std::cout << "ALL ASIO TESTS PASSED!" << std::endl;
        std::cout << "ASIO integration is working correctly!" << std::endl;
    }
    else {
        std::cout << "Some tests failed - check ASIO setup" << std::endl;
    }
    std::cout << std::string(50, '=') << std::endl;
}

// Test ASIO interface creation
bool testASIOInterfaceCreation() {
    std::cout << "\nTesting ASIO interface creation..." << std::endl;

    try {
        auto asio_interface = std::make_unique<ASIOInterface>();
        addTestResult(true, "ASIO interface creation", "Interface created successfully");
        return true;
    }
    catch (const std::exception& e) {
        addTestResult(false, "ASIO interface creation", std::string("Exception: ") + e.what());
        return false;
    }
}

// Test ASIO driver enumeration
bool testASIODriverEnumeration() {
    std::cout << "\nTesting ASIO driver enumeration..." << std::endl;

    try {
        auto asio_interface = std::make_unique<ASIOInterface>();
        auto drivers = asio_interface->getAvailableDrivers();

        std::cout << "  Found " << drivers.size() << " ASIO driver(s):" << std::endl;
        for (const auto& driver : drivers) {
            std::cout << "    - " << driver << std::endl;
        }

        if (drivers.empty()) {
            addTestResult(true, "ASIO driver enumeration", "No drivers found (normal if no ASIO hardware)");
        }
        else {
            addTestResult(true, "ASIO driver enumeration",
                std::to_string(drivers.size()) + " driver(s) found");
        }
        return true;
    }
    catch (const std::exception& e) {
        addTestResult(false, "ASIO driver enumeration", std::string("Exception: ") + e.what());
        return false;
    }
}

// Test ASIO hardware detection
bool testASIOHardwareDetection() {
    std::cout << "\nTesting ASIO hardware detection..." << std::endl;

    try {
        auto asio_interface = std::make_unique<ASIOInterface>();
        auto hardware_types = asio_interface->detectHardwareTypes();

        std::cout << "  Detected " << hardware_types.size() << " device type(s):" << std::endl;
        for (const auto& hw_type : hardware_types) {
            std::cout << "    - " << hardwareTypeToString(hw_type) << std::endl;
        }

        addTestResult(true, "ASIO hardware detection",
            std::to_string(hardware_types.size()) + " device type(s) detected");
        return true;
    }
    catch (const std::exception& e) {
        addTestResult(false, "ASIO hardware detection", std::string("Exception: ") + e.what());
        return false;
    }
}

// Test ASIO interface initialization
bool testASIOInitialization() {
    std::cout << "\nTesting ASIO interface initialization..." << std::endl;

    try {
        auto asio_interface = std::make_unique<ASIOInterface>();

        bool init_result = asio_interface->initialize(TEST_SAMPLE_RATE, TEST_BUFFER_SIZE);

        if (init_result) {
            addTestResult(true, "ASIO interface initialization", "Initialization successful");

            // Test hardware info retrieval
            std::cout << "  Hardware type: " << hardwareTypeToString(asio_interface->getType()) << std::endl;
            std::cout << "  Hardware name: " << asio_interface->getName() << std::endl;
            std::cout << "  Input channels: " << asio_interface->getInputChannelCount() << std::endl;
            std::cout << "  Output channels: " << asio_interface->getOutputChannelCount() << std::endl;
            std::cout << "  Latency: " << asio_interface->getCurrentLatency() << "ms" << std::endl;

            // Clean shutdown
            asio_interface->shutdown();
            return true;
        }
        else {
            addTestResult(false, "ASIO interface initialization", "Initialization failed");
            return false;
        }
    }
    catch (const std::exception& e) {
        addTestResult(false, "ASIO interface initialization", std::string("Exception: ") + e.what());
        return false;
    }
}

// Test ASIO streaming
bool testASIOStreaming() {
    std::cout << "\nTesting ASIO streaming..." << std::endl;

    try {
        auto asio_interface = std::make_unique<ASIOInterface>();

        // Initialize interface
        if (!asio_interface->initialize(TEST_SAMPLE_RATE, TEST_BUFFER_SIZE)) {
            addTestResult(false, "ASIO streaming test", "Failed to initialize interface");
            return false;
        }

        // Create test processor using your factory function
        auto processor = createTestProcessor(false);  // No tone generation for test

        // Start streaming
        bool start_result = asio_interface->startStreaming(processor.get());
        if (!start_result) {
            addTestResult(false, "ASIO streaming test", "Failed to start streaming");
            asio_interface->shutdown();
            return false;
        }

        std::cout << "  Streaming started, testing for " << TEST_DURATION_MS << "ms..." << std::endl;

        // Check streaming status
        if (!asio_interface->isStreaming()) {
            addTestResult(false, "ASIO streaming test", "Interface reports not streaming");
            asio_interface->shutdown();
            return false;
        }

        // Get metrics
        auto metrics = asio_interface->getMetrics();
        std::cout << "  Latency: " << metrics.latency_ms << "ms" << std::endl;
        std::cout << "  CPU Usage: " << metrics.cpu_usage_percent << "%" << std::endl;

        // Run for test duration
        std::this_thread::sleep_for(std::chrono::milliseconds(TEST_DURATION_MS));

        // Stop streaming
        asio_interface->stopStreaming();

        // Verify stopped
        if (asio_interface->isStreaming()) {
            addTestResult(false, "ASIO streaming test", "Interface still reports streaming after stop");
            asio_interface->shutdown();
            return false;
        }

        std::cout << "  Streaming stopped successfully" << std::endl;

        // Clean shutdown
        asio_interface->shutdown();

        addTestResult(true, "ASIO streaming test",
            "Streaming worked for " + std::to_string(TEST_DURATION_MS) + "ms");
        return true;

    }
    catch (const std::exception& e) {
        addTestResult(false, "ASIO streaming test", std::string("Exception: ") + e.what());
        return false;
    }
}

// Test integration with core audio interface functions
bool testCoreIntegration() {
    std::cout << "\nTesting core audio interface integration..." << std::endl;

    try {
        // Test hardware detection through core functions
        auto hardware_types = detectAvailableHardware();
        std::cout << "  Core detection found " << hardware_types.size() << " device type(s)" << std::endl;

        if (hardware_types.empty()) {
            addTestResult(false, "Core integration test", "No hardware detected");
            return false;
        }

        // Test interface creation through core functions
        auto interface = createAudioInterface(hardware_types[0]);
        if (!interface) {
            addTestResult(false, "Core integration test", "Failed to create interface");
            return false;
        }

        std::cout << "  Interface created through core functions" << std::endl;

        // Test basic operations
        if (!interface->initialize(TEST_SAMPLE_RATE, TEST_BUFFER_SIZE)) {
            addTestResult(false, "Core integration test", "Failed to initialize through core");
            return false;
        }

        auto processor = createTestProcessor(false);
        if (!interface->startStreaming(processor.get())) {
            addTestResult(false, "Core integration test", "Failed to start streaming through core");
            interface->shutdown();
            return false;
        }

        // Brief test
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        interface->stopStreaming();
        interface->shutdown();

        addTestResult(true, "Core integration test", "ASIO integrates correctly with core functions");
        return true;

    }
    catch (const std::exception& e) {
        addTestResult(false, "Core integration test", std::string("Exception: ") + e.what());
        return false;
    }
}

// Test graceful fallback when ASIO not available
bool testGracefulFallback() {
    std::cout << "\nTesting graceful fallback behavior..." << std::endl;

    try {
        // This test ensures the system works even without ASIO hardware
        auto interface = createStubInterface();
        if (!interface) {
            addTestResult(false, "Graceful fallback test", "Failed to create fallback interface");
            return false;
        }

        if (!interface->initialize()) {
            addTestResult(false, "Graceful fallback test", "Failed to initialize fallback interface");
            return false;
        }

        auto processor = createTestProcessor(false);
        if (!interface->startStreaming(processor.get())) {
            addTestResult(false, "Graceful fallback test", "Failed to start fallback streaming");
            interface->shutdown();
            return false;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        interface->stopStreaming();
        interface->shutdown();

        addTestResult(true, "Graceful fallback test", "System provides working fallback");
        return true;

    }
    catch (const std::exception& e) {
        addTestResult(false, "Graceful fallback test", std::string("Exception: ") + e.what());
        return false;
    }
}

int main() {
    std::cout << std::string(60, '=') << std::endl;
    std::cout << "SYNTRI ASIO INTEGRATION TEST" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    std::cout << "Testing ASIO integration with real hardware detection" << std::endl;
    std::cout << "Sample Rate: " << TEST_SAMPLE_RATE << " Hz" << std::endl;
    std::cout << "Buffer Size: " << TEST_BUFFER_SIZE << " samples" << std::endl;
    std::cout << "Test Duration: " << TEST_DURATION_MS << "ms per streaming test" << std::endl;
    std::cout << std::string(60, '=') << std::endl;

    // Run all tests
    bool all_passed = true;

    all_passed &= testASIOInterfaceCreation();
    all_passed &= testASIODriverEnumeration();
    all_passed &= testASIOHardwareDetection();
    all_passed &= testASIOInitialization();
    all_passed &= testASIOStreaming();
    all_passed &= testCoreIntegration();
    all_passed &= testGracefulFallback();

    // Print final summary
    printTestSummary();

    // Return appropriate exit code
    return all_passed ? 0 : 1;
}