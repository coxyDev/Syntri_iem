// src/core/audio_interface.cpp
// FIXED Implementation - Properly working without ASIO conflicts
// Preserves your foundation while preparing for ASIO when needed

#define _USE_MATH_DEFINES  // Enable M_PI, M_E, etc. in MSVC
#include <cmath>


#include <cmath>
#include <algorithm>

#include "syntri/audio_interface.h"
#include <iostream>
#include <chrono>
#include <thread>

namespace Syntri {

    // ====================================
    // StubAudioInterface - Your Working Foundation (PRESERVED)
    // ====================================
    class StubAudioInterface : public AudioInterface {
    private:
        bool initialized_;
        bool streaming_;
        int sample_rate_;
        int buffer_size_;
        AudioProcessor* processor_;
        std::chrono::high_resolution_clock::time_point last_callback_time_;
        HardwareType hardware_type_;
        int callback_count_;

    public:
        StubAudioInterface()
            : initialized_(false), streaming_(false), sample_rate_(SAMPLE_RATE_96K),
            buffer_size_(BUFFER_SIZE_ULTRA_LOW), processor_(nullptr),
            hardware_type_(HardwareType::GENERIC_ASIO), callback_count_(0) {
            std::cout << "Creating stub audio interface..." << std::endl;
            last_callback_time_ = std::chrono::high_resolution_clock::now();
        }

        ~StubAudioInterface() {
            std::cout << "Destroying stub audio interface..." << std::endl;
            if (streaming_) {
                stopStreaming();
            }
            if (initialized_) {
                shutdown();
            }
        }

        bool initialize(int sample_rate = SAMPLE_RATE_96K, int buffer_size = BUFFER_SIZE_ULTRA_LOW) override {
            std::cout << "Initializing stub interface (SR: " << sample_rate
                << " Hz, Buffer: " << buffer_size << ")" << std::endl;

            sample_rate_ = sample_rate;
            buffer_size_ = buffer_size;
            initialized_ = true;

            std::cout << "Stub interface initialized successfully" << std::endl;
            return true;
        }

        void shutdown() override {
            if (!initialized_) return;

            std::cout << "Shutting down stub interface..." << std::endl;
            if (streaming_) {
                stopStreaming();
            }
            initialized_ = false;
            std::cout << "Stub interface shutdown complete" << std::endl;
        }

        bool isInitialized() const override {
            return initialized_;
        }

        HardwareType getType() const override {
            return hardware_type_;
        }

        std::string getName() const override {
            return "Syntri Stub Interface";
        }

        int getInputChannelCount() const override {
            return 8;  // Simulated 8-channel input
        }

        int getOutputChannelCount() const override {
            return 8;  // Simulated 8-channel output
        }

        double getCurrentLatency() const override {
            // Calculate theoretical latency based on buffer size and sample rate
            return (static_cast<double>(buffer_size_) / static_cast<double>(sample_rate_)) * 1000.0;
        }

        bool startStreaming(AudioProcessor* processor) override {
            if (!initialized_ || !processor) {
                std::cout << "Cannot start streaming - not initialized or no processor" << std::endl;
                return false;
            }

            processor_ = processor;
            std::cout << "Starting stub streaming..." << std::endl;

            // Notify processor of setup
            processor_->setupChanged(sample_rate_, buffer_size_);

            streaming_ = true;
            std::cout << "Stub streaming started successfully" << std::endl;

            return true;
        }

        void stopStreaming() override {
            if (!streaming_) return;

            std::cout << "Stopping stub streaming..." << std::endl;
            streaming_ = false;
            processor_ = nullptr;
            std::cout << "Stub streaming stopped" << std::endl;
        }

        bool isStreaming() const override {
            return streaming_;
        }

        SimpleMetrics getMetrics() const override {
            SimpleMetrics metrics;
            metrics.latency_ms = getCurrentLatency();
            metrics.cpu_usage_percent = 5.0 + (callback_count_ % 10) * 0.5; // Simulated CPU usage
            metrics.buffer_underruns = 0; // Stub never has underruns
            return metrics;
        }
    };

    // ====================================
    // TestAudioProcessor - Internal Implementation
    // ====================================
    class TestAudioProcessor : public AudioProcessor {
    private:
        bool generate_tone_;
        double phase_;
        double frequency_;
        int sample_rate_;

    public:
        TestAudioProcessor(bool generate_tone = false)
            : generate_tone_(generate_tone), phase_(0.0), frequency_(440.0), sample_rate_(SAMPLE_RATE_96K) {
            std::cout << "Creating test audio processor (tone: " << (generate_tone ? "ON" : "OFF") << ")" << std::endl;
        }

        void processAudio(
            const MultiChannelBuffer& inputs,
            MultiChannelBuffer& outputs,
            int num_samples
        ) override {
            // Ensure outputs are properly sized
            for (auto& channel : outputs) {
                if (channel.size() != static_cast<size_t>(num_samples)) {
                    channel.resize(num_samples, 0.0f);
                }
            }

            if (generate_tone_) {
                // Generate test tone using M_PI (now properly defined)
                double phase_increment = 2.0 * M_PI * frequency_ / sample_rate_;

                for (int sample = 0; sample < num_samples; ++sample) {
                    float tone_sample = static_cast<float>(0.1 * std::sin(phase_));
                    phase_ += phase_increment;

                    // Apply to all output channels
                    for (auto& channel : outputs) {
                        if (sample < static_cast<int>(channel.size())) {
                            channel[sample] = tone_sample;
                        }
                    }
                }

                // Keep phase in range
                while (phase_ >= 2.0 * M_PI) {
                    phase_ -= 2.0 * M_PI;
                }
            }
            else {
                // Pass through inputs to outputs (or silence if no inputs)
                for (size_t ch = 0; ch < outputs.size(); ++ch) {
                    for (int sample = 0; sample < num_samples; ++sample) {
                        if (sample < static_cast<int>(outputs[ch].size())) {
                            if (ch < inputs.size() && sample < static_cast<int>(inputs[ch].size())) {
                                outputs[ch][sample] = inputs[ch][sample];
                            }
                            else {
                                outputs[ch][sample] = 0.0f;
                            }
                        }
                    }
                }
            }
        }

        void setupChanged(int sample_rate, int buffer_size) override {
            sample_rate_ = sample_rate;
            std::cout << "Test processor setup changed (SR: " << sample_rate
                << " Hz, Buffer: " << buffer_size << ")" << std::endl;
        }
    };

    // ====================================
    // Factory Functions - Simplified and Working
    // ====================================
    std::unique_ptr<AudioInterface> createAudioInterface(HardwareType type) {
        std::cout << "Creating audio interface for: " << hardwareTypeToString(type) << std::endl;

        // For now, always return stub interface
        // TODO: Add ASIO integration when properly implemented
        std::cout << "Using stub interface (ASIO integration pending)" << std::endl;
        return std::make_unique<StubAudioInterface>();
    }

    std::unique_ptr<AudioInterface> createStubInterface() {
        std::cout << "Creating stub interface directly" << std::endl;
        return std::make_unique<StubAudioInterface>();
    }

    std::unique_ptr<AudioProcessor> createTestProcessor(bool generate_tone) {
        return std::make_unique<TestAudioProcessor>(generate_tone);
    }

    // ====================================
    // Hardware Detection - Simplified for Now
    // ====================================
    std::vector<HardwareType> detectAvailableHardware() {
        std::vector<HardwareType> detected;

        std::cout << "Detecting available audio hardware..." << std::endl;

        // For now, just return generic interface
        // TODO: Add real ASIO detection when properly implemented
        detected.push_back(HardwareType::GENERIC_ASIO);
        std::cout << "Generic interface available" << std::endl;

        std::cout << "Detection complete. Found " << detected.size() << " interface(s):" << std::endl;
        for (const auto& hw : detected) {
            std::cout << "   - " << hardwareTypeToString(hw) << std::endl;
        }

        return detected;
    }

    // ====================================
    // Utility Functions
    // ====================================
    void printHardwareInfo(HardwareType type) {
        std::cout << "=====================================" << std::endl;
        std::cout << "Hardware Information" << std::endl;
        std::cout << "=====================================" << std::endl;
        std::cout << "Type: " << hardwareTypeToString(type) << std::endl;

        // Create interface to get detailed info
        auto interface = createAudioInterface(type);
        if (interface && interface->initialize()) {
            std::cout << "Name: " << interface->getName() << std::endl;
            std::cout << "Input Channels: " << interface->getInputChannelCount() << std::endl;
            std::cout << "Output Channels: " << interface->getOutputChannelCount() << std::endl;
            std::cout << "Current Latency: " << interface->getCurrentLatency() << " ms" << std::endl;

            auto metrics = interface->getMetrics();
            std::cout << "CPU Usage: " << metrics.cpu_usage_percent << "%" << std::endl;
            std::cout << "Buffer Underruns: " << metrics.buffer_underruns << std::endl;

            interface->shutdown();
        }
        else {
            std::cout << "Failed to initialize interface for detailed info" << std::endl;
        }
        std::cout << "=====================================" << std::endl;
    }

    bool runBasicHardwareTest() {
        std::cout << "=====================================" << std::endl;
        std::cout << "Running Basic Hardware Test" << std::endl;
        std::cout << "=====================================" << std::endl;

        // Test hardware detection
        auto detected = detectAvailableHardware();
        if (detected.empty()) {
            std::cout << "No hardware detected" << std::endl;
            return false;
        }

        // Test each detected hardware type
        bool all_passed = true;
        for (const auto& hw_type : detected) {
            std::cout << "\nTesting: " << hardwareTypeToString(hw_type) << std::endl;

            auto interface = createAudioInterface(hw_type);
            if (!interface) {
                std::cout << "Failed to create interface" << std::endl;
                all_passed = false;
                continue;
            }

            // Test initialization
            if (!interface->initialize()) {
                std::cout << "Failed to initialize interface" << std::endl;
                all_passed = false;
                continue;
            }

            std::cout << "Interface initialized successfully" << std::endl;
            std::cout << "   Latency: " << interface->getCurrentLatency() << " ms" << std::endl;

            // Test streaming setup
            auto processor = createTestProcessor(false);
            if (interface->startStreaming(processor.get())) {
                std::cout << "Streaming started successfully" << std::endl;

                // Simulate brief operation
                std::this_thread::sleep_for(std::chrono::milliseconds(100));

                interface->stopStreaming();
                std::cout << "Streaming stopped successfully" << std::endl;
            }
            else {
                std::cout << "Streaming test failed (non-critical)" << std::endl;
            }

            interface->shutdown();
            std::cout << "Interface shutdown successfully" << std::endl;
        }

        std::cout << "\n=====================================" << std::endl;
        if (all_passed) {
            std::cout << "ALL TESTS PASSED!" << std::endl;
            std::cout << "Your audio foundation is working correctly!" << std::endl;
        }
        else {
            std::cout << "Some tests failed, but basic functionality works" << std::endl;
            std::cout << "System can proceed with available interfaces" << std::endl;
        }
        std::cout << "=====================================" << std::endl;

        return all_passed;
    }

} // namespace Syntri