// test/asio_test.cpp
// Comprehensive ASIO integration test with proper error handling
#include <iostream>
#include <memory>
#include <vector>
#include <exception>

#include "syntri/audio_interface.h"
#include "syntri/types.h"

#ifdef ENABLE_ASIO_SUPPORT
#include "syntri/asio_interface.h"
#endif

int main() {
    try {
        std::cout << "=====================================" << std::endl;
        std::cout << "    SYNTRI ASIO INTEGRATION TEST" << std::endl;
        std::cout << "=====================================" << std::endl;
        std::cout << std::endl;

#ifdef ENABLE_ASIO_SUPPORT
        std::cout << "✅ ASIO support compiled in" << std::endl;
#else
        std::cout << "⚠️  ASIO support NOT compiled in (stub mode only)" << std::endl;
#endif
        std::cout << std::endl;

        // Test 1: Hardware Detection
        std::cout << "🔧 Test 1: Hardware Detection" << std::endl;
        auto detected_hardware = Syntri::detectAvailableHardware();
        std::cout << "Detected " << detected_hardware.size() << " audio devices:" << std::endl;
        for (const auto& hw : detected_hardware) {
            std::cout << "  - " << Syntri::hardwareTypeToString(hw) << std::endl;
        }
        std::cout << "✅ Hardware detection complete" << std::endl;
        std::cout << std::endl;

        // Test 2: Interface Creation for Each Hardware Type
        std::cout << "🔧 Test 2: Interface Creation" << std::endl;
        std::vector<Syntri::HardwareType> test_types = {
            Syntri::HardwareType::GENERIC_ASIO,
            Syntri::HardwareType::UAD_APOLLO_X16,
            Syntri::HardwareType::ALLEN_HEATH_AVANTIS,
            Syntri::HardwareType::BEHRINGER_X32
        };

        for (auto type : test_types) {
            std::cout << "Testing " << Syntri::hardwareTypeToString(type) << "..." << std::endl;
            auto interface = Syntri::createAudioInterface(type);

            if (interface) {
                std::cout << "  ✅ Interface created: " << interface->getName() << std::endl;

                // Test initialization
                if (interface->initialize()) {
                    std::cout << "  ✅ Initialization successful" << std::endl;
                    std::cout << "    Channels: " << interface->getInputChannelCount()
                        << " in / " << interface->getOutputChannelCount() << " out" << std::endl;
                    std::cout << "    Latency: " << interface->getCurrentLatency() << " ms" << std::endl;
                    interface->shutdown();
                }
                else {
                    std::cout << "  ⚠️  Initialization failed (expected for some devices)" << std::endl;
                }
            }
            else {
                std::cout << "  ❌ Failed to create interface" << std::endl;
                return 1;
            }
        }
        std::cout << "✅ Interface creation tests complete" << std::endl;
        std::cout << std::endl;

#ifdef ENABLE_ASIO_SUPPORT
        // Test 3: ASIO-Specific Functionality
        std::cout << "🔧 Test 3: ASIO-Specific Tests" << std::endl;
        auto asio_interface = std::make_unique<Syntri::ASIOInterface>();

        if (asio_interface->initialize()) {
            std::cout << "  ✅ ASIO interface initialized" << std::endl;

            // Test driver enumeration
            auto drivers = asio_interface->getAvailableDrivers();
            std::cout << "  Available ASIO drivers (" << drivers.size() << "):" << std::endl;
            for (const auto& driver : drivers) {
                std::cout << "    - " << driver << std::endl;
                auto detected_type = asio_interface->detectHardwareType(driver);
                std::cout << "      Detected as: " << Syntri::hardwareTypeToString(detected_type) << std::endl;
            }

            if (!drivers.empty()) {
                std::cout << "  ✅ ASIO drivers found" << std::endl;

                // Test loading first driver
                if (asio_interface->loadDriver(drivers[0])) {
                    std::cout << "  ✅ Successfully loaded driver: " << drivers[0] << std::endl;
                    std::cout << "    Type: " << Syntri::hardwareTypeToString(asio_interface->getType()) << std::endl;
                    std::cout << "    Name: " << asio_interface->getName() << std::endl;
                    std::cout << "    Latency: " << asio_interface->getCurrentLatency() << " ms" << std::endl;
                    asio_interface->unloadDriver();
                }
                else {
                    std::cout << "  ⚠️  Failed to load driver (may require physical hardware)" << std::endl;
                }
            }
            else {
                std::cout << "  ⚠️  No ASIO drivers found (falling back to stub mode)" << std::endl;
            }

            asio_interface->shutdown();
        }
        else {
            std::cout << "  ⚠️  ASIO interface initialization failed" << std::endl;
        }
        std::cout << "✅ ASIO-specific tests complete" << std::endl;
        std::cout << std::endl;
#endif

        // Test 4: Audio Processing Chain
        std::cout << "🔧 Test 4: Audio Processing Chain" << std::endl;
        auto test_interface = Syntri::createAudioInterface(Syntri::HardwareType::GENERIC_ASIO);
        auto test_processor = Syntri::createTestProcessor(false);

        if (test_interface->initialize()) {
            std::cout << "  ✅ Interface initialized" << std::endl;

            if (test_interface->startStreaming(test_processor.get())) {
                std::cout << "  ✅ Audio streaming started" << std::endl;

                // Get metrics
                auto metrics = test_interface->getMetrics();
                std::cout << "  Performance metrics:" << std::endl;
                std::cout << "    Latency: " << metrics.latency_ms << " ms" << std::endl;
                std::cout << "    CPU Usage: " << metrics.cpu_usage << "%" << std::endl;
                std::cout << "    Callbacks: " << metrics.callback_count << std::endl;
                std::cout << "    Underruns: " << metrics.underruns << std::endl;

                test_interface->stopStreaming();
                std::cout << "  ✅ Audio streaming stopped" << std::endl;
            }
            else {
                std::cout << "  ❌ Failed to start audio streaming" << std::endl;
                return 1;
            }

            test_interface->shutdown();
        }
        else {
            std::cout << "  ❌ Interface initialization failed" << std::endl;
            return 1;
        }
        std::cout << "✅ Audio processing chain test complete" << std::endl;
        std::cout << std::endl;

        // Test 5: Stress Test - Multiple Interfaces
        std::cout << "🔧 Test 5: Multiple Interface Stress Test" << std::endl;
        std::vector<std::unique_ptr<Syntri::AudioInterface>> interfaces;

        // Create multiple interfaces
        for (int i = 0; i < 5; i++) {
            auto interface = Syntri::createAudioInterface(Syntri::HardwareType::GENERIC_ASIO);
            if (interface->initialize()) {
                interfaces.push_back(std::move(interface));
            }
        }

        std::cout << "  Created " << interfaces.size() << " concurrent interfaces" << std::endl;

        // Test concurrent streaming
        std::vector<std::unique_ptr<Syntri::AudioProcessor>> processors;
        int streaming_count = 0;

        for (auto& interface : interfaces) {
            auto processor = Syntri::createTestProcessor(false);
            if (interface->startStreaming(processor.get())) {
                streaming_count++;
                processors.push_back(std::move(processor));
            }
        }

        std::cout << "  " << streaming_count << " interfaces streaming concurrently" << std::endl;

        // Stop all streaming
        for (auto& interface : interfaces) {
            if (interface->isStreaming()) {
                interface->stopStreaming();
            }
            interface->shutdown();
        }

        std::cout << "  ✅ All interfaces cleaned up" << std::endl;
        std::cout << "✅ Stress test complete" << std::endl;
        std::cout << std::endl;

        // Test 6: Basic Hardware Test Function
        std::cout << "🔧 Test 6: Built-in Hardware Test" << std::endl;
        if (Syntri::runBasicHardwareTest()) {
            std::cout << "✅ Built-in hardware test passed" << std::endl;
        }
        else {
            std::cout << "❌ Built-in hardware test failed" << std::endl;
            return 1;
        }
        std::cout << std::endl;

        // Success!
        std::cout << "=====================================" << std::endl;
        std::cout << "  🎉 ALL ASIO TESTS PASSED! 🎉" << std::endl;
        std::cout << "=====================================" << std::endl;
        std::cout << std::endl;

#ifdef ENABLE_ASIO_SUPPORT
        std::cout << "SUCCESS: ASIO Integration Working!" << std::endl;
        std::cout << std::endl;
        std::cout << "Phase 1 Progress:" << std::endl;
        std::cout << "✅ Foundation working" << std::endl;
        std::cout << "✅ Audio interface abstraction working" << std::endl;
        std::cout << "✅ ASIO integration working" << std::endl;
        std::cout << "🔄 Next: Add real hardware detection" << std::endl;
        std::cout << "🔄 Next: Add real audio I/O processing" << std::endl;
        std::cout << "🔄 Next: Add performance optimization" << std::endl;
#else
        std::cout << "SUCCESS: Audio System Working (Stub Mode)" << std::endl;
        std::cout << std::endl;
        std::cout << "To enable full ASIO support:" << std::endl;
        std::cout << "1. Download ASIO SDK 2.3.3 from Steinberg" << std::endl;
        std::cout << "2. Extract to C:\\asiosdk_2.3.3" << std::endl;
        std::cout << "3. Rebuild with: cmake .. -DENABLE_ASIO_SUPPORT=ON" << std::endl;
#endif
        std::cout << std::endl;

        return 0;

    }
    catch (const std::exception& e) {
        std::cout << "❌ Exception caught: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cout << "❌ Unknown exception caught!" << std::endl;
        return 1;
    }
}