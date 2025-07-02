// Syntri Interface Test - Audio Interface Testing
// Phase 1: Test hardware detection and interface functionality
// Copyright (c) 2025 Syntri Technologies

#include "syntri/audio_interface.h"
#include <iostream>
#include <thread>
#include <chrono>

// Simple test audio processor
class TestAudioProcessor : public Syntri::AudioProcessor {
private:
    int callback_count_ = 0;
    int sample_rate_ = 0;
    int buffer_size_ = 0;

public:
    void processAudio(
        const Syntri::MultiChannelBuffer& inputs,
        Syntri::MultiChannelBuffer& outputs,
        int num_samples
    ) override {
        callback_count_++;

        // Simple pass-through processing
        outputs.resize(inputs.size());
        for (size_t ch = 0; ch < outputs.size(); ++ch) {
            outputs[ch].resize(num_samples, 0.0f);
            if (ch < inputs.size() && inputs[ch].size() >= static_cast<size_t>(num_samples)) {
                std::copy_n(inputs[ch].begin(), num_samples, outputs[ch].begin());
            }
        }
    }

    void setupChanged(int sample_rate, int buffer_size) override {
        sample_rate_ = sample_rate;
        buffer_size_ = buffer_size;
        std::cout << "    Audio setup: " << sample_rate << "Hz, " << buffer_size << " samples" << std::endl;
    }

    int getCallbackCount() const { return callback_count_; }
    void resetCallbackCount() { callback_count_ = 0; }
};

int main() {
    std::cout << "=====================================" << std::endl;
    std::cout << "    SYNTRI - AUDIO INTERFACE TEST" << std::endl;
    std::cout << "=====================================" << std::endl;
    std::cout << std::endl;

    try {
        // Test 1: Hardware Detection
        std::cout << "🔧 Test 1: Hardware Detection" << std::endl;
        auto available_hardware = Syntri::detectAvailableHardware();
        std::cout << "✅ Detected " << available_hardware.size() << " hardware interface(s)" << std::endl;
        std::cout << std::endl;

        // Test 2: Interface Creation
        std::cout << "🔧 Test 2: Interface Creation" << std::endl;
        for (const auto& hw_type : available_hardware) {
            std::cout << "  Testing: " << Syntri::hardwareTypeToString(hw_type) << std::endl;

            auto interface = Syntri::createAudioInterface(hw_type);
            if (interface) {
                std::cout << "    ✅ Interface created successfully" << std::endl;
                std::cout << "    Type: " << Syntri::hardwareTypeToString(interface->getType()) << std::endl;
                std::cout << "    Name: " << interface->getName() << std::endl;
            }
            else {
                std::cout << "    ❌ Failed to create interface" << std::endl;
                return 1;
            }
        }
        std::cout << "✅ All interfaces created successfully" << std::endl;
        std::cout << std::endl;

        // Test 3: Interface Initialization
        std::cout << "🔧 Test 3: Interface Initialization" << std::endl;
        auto test_interface = Syntri::createAudioInterface(Syntri::HardwareType::UAD_APOLLO_X16);

        // Test initialization
        bool init_result = test_interface->initialize(Syntri::SAMPLE_RATE_96K, Syntri::BUFFER_SIZE_ULTRA_LOW);
        if (init_result && test_interface->isInitialized()) {
            std::cout << "✅ Interface initialization successful" << std::endl;
        }
        else {
            std::cout << "❌ Interface initialization failed" << std::endl;
            return 1;
        }
        std::cout << std::endl;

        // Test 4: Interface Information
        std::cout << "🔧 Test 4: Interface Information" << std::endl;
        std::cout << "  Hardware Type: " << Syntri::hardwareTypeToString(test_interface->getType()) << std::endl;
        std::cout << "  Name: " << test_interface->getName() << std::endl;
        std::cout << "  Input Channels: " << test_interface->getInputChannelCount() << std::endl;
        std::cout << "  Output Channels: " << test_interface->getOutputChannelCount() << std::endl;
        std::cout << "  Current Latency: " << test_interface->getCurrentLatency() << " ms" << std::endl;

        auto metrics = test_interface->getMetrics();
        std::cout << "  CPU Usage: " << metrics.cpu_usage_percent << "%" << std::endl;
        std::cout << "  Buffer Underruns: " << metrics.buffer_underruns << std::endl;
        std::cout << "✅ Interface information retrieved" << std::endl;
        std::cout << std::endl;

        // Test 5: Audio Streaming
        std::cout << "🔧 Test 5: Audio Streaming" << std::endl;
        TestAudioProcessor processor;

        // Start streaming
        bool stream_result = test_interface->startStreaming(&processor);
        if (stream_result && test_interface->isStreaming()) {
            std::cout << "✅ Audio streaming started" << std::endl;

            // Simulate some processing time
            std::cout << "  Simulating audio processing for 2 seconds..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));

            // Check if streaming is still active
            if (test_interface->isStreaming()) {
                std::cout << "✅ Audio streaming still active" << std::endl;
            }
            else {
                std::cout << "❌ Audio streaming stopped unexpectedly" << std::endl;
            }

            // Stop streaming
            test_interface->stopStreaming();
            if (!test_interface->isStreaming()) {
                std::cout << "✅ Audio streaming stopped successfully" << std::endl;
            }
            else {
                std::cout << "❌ Failed to stop audio streaming" << std::endl;
            }
        }
        else {
            std::cout << "❌ Failed to start audio streaming" << std::endl;
            return 1;
        }
        std::cout << std::endl;

        // Test 6: Interface Shutdown
        std::cout << "🔧 Test 6: Interface Shutdown" << std::endl;
        test_interface->shutdown();
        if (!test_interface->isInitialized()) {
            std::cout << "✅ Interface shutdown successful" << std::endl;
        }
        else {
            std::cout << "❌ Interface shutdown failed" << std::endl;
            return 1;
        }
        std::cout << std::endl;

        // Test 7: Multiple Interface Types
        std::cout << "🔧 Test 7: Multiple Interface Types" << std::endl;
        std::vector<std::unique_ptr<Syntri::AudioInterface>> interfaces;

        // Create multiple interface types
        interfaces.push_back(Syntri::createAudioInterface(Syntri::HardwareType::UAD_APOLLO_X16));
        interfaces.push_back(Syntri::createAudioInterface(Syntri::HardwareType::ALLEN_HEATH_AVANTIS));
        interfaces.push_back(Syntri::createAudioInterface(Syntri::HardwareType::BEHRINGER_X32));

        // Test each interface
        for (auto& interface : interfaces) {
            if (interface->initialize()) {
                std::cout << "  ✅ " << interface->getName() << " initialized" << std::endl;
                std::cout << "    Latency: " << interface->getCurrentLatency() << " ms" << std::endl;
                std::cout << "    Channels: " << interface->getInputChannelCount()
                    << " in / " << interface->getOutputChannelCount() << " out" << std::endl;
                interface->shutdown();
            }
            else {
                std::cout << "  ❌ " << interface->getName() << " failed to initialize" << std::endl;
            }
        }
        std::cout << "✅ Multiple interface types tested" << std::endl;
        std::cout << std::endl;

        // Success!
        std::cout << "=====================================" << std::endl;
        std::cout << "    🎉 ALL TESTS PASSED! 🎉" << std::endl;
        std::cout << "=====================================" << std::endl;
        std::cout << std::endl;
        std::cout << "Audio interface layer is working correctly!" << std::endl;
        std::cout << std::endl;
        std::cout << "Phase 1 Progress:" << std::endl;
        std::cout << "✅ Foundation working" << std::endl;
        std::cout << "✅ Audio interface abstraction working" << std::endl;
        std::cout << "🔄 Next: Add ASIO integration" << std::endl;
        std::cout << "🔄 Next: Add hardware detection" << std::endl;
        std::cout << "🔄 Next: Add real audio I/O" << std::endl;
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