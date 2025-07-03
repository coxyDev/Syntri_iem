// src/core/audio_interface.cpp
// Fixed implementation with proper inheritance and type handling
#include "syntri/audio_interface.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <cmath>

#ifdef ENABLE_ASIO_SUPPORT
#include "syntri/asio_interface.h"
#endif

namespace Syntri {

    // ====================================
    // StubAudioInterface - Internal Implementation
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

            std::cout << "Stub interface initialized" << std::endl;
            return true;
        }

        void shutdown() override {
            std::cout << "Shutting down stub interface..." << std::endl;
            if (streaming_) {
                stopStreaming();
            }
            initialized_ = false;
            std::cout << "Stub interface shut down" << std::endl;
        }

        bool isInitialized() const override {
            return initialized_;
        }

        HardwareType getType() const override {
            return hardware_type_;
        }

        std::string getName() const override {
            return "Stub Audio Interface";
        }

        int getInputChannelCount() const override {
            return 2;  // Stereo
        }

        int getOutputChannelCount() const override {
            return 2;  // Stereo
        }

        double getCurrentLatency() const override {
            if (buffer_size_ > 0 && sample_rate_ > 0) {
                return (static_cast<double>(buffer_size_) / static_cast<double>(sample_rate_)) * 1000.0;
            }
            return 0.0;
        }

        bool startStreaming(AudioProcessor* processor) override {
            std::cout << "Starting stub streaming..." << std::endl;

            if (!initialized_) {
                std::cout << "Stub interface not initialized" << std::endl;
                return false;
            }

            if (streaming_) {
                std::cout << "Stub already streaming" << std::endl;
                return true;
            }

            processor_ = processor;
            streaming_ = true;

            if (processor_) {
                processor_->setupChanged(sample_rate_, buffer_size_);

                // Simulate a few audio callbacks for testing
                MultiChannelBuffer inputs(2, std::vector<float>(buffer_size_, 0.0f));
                MultiChannelBuffer outputs(2, std::vector<float>(buffer_size_, 0.0f));

                for (int i = 0; i < 3; i++) {
                    processor_->processAudio(inputs, outputs, buffer_size_);
                    callback_count_++;
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            }

            std::cout << "Stub streaming started" << std::endl;
            return true;
        }

        void stopStreaming() override {
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
            metrics.cpu_usage_percent = 15.0;  // Simulated
            metrics.buffer_underruns = 0;
            return metrics;
        }
    };

    // ====================================
    // TestAudioProcessor Implementation
    // ====================================

    class TestAudioProcessor : public AudioProcessor {
    private:
        bool generate_tone_;
        float phase_;
        float frequency_;
        int sample_rate_;

    public:
        TestAudioProcessor(bool generate_tone = false)
            : generate_tone_(generate_tone), phase_(0.0f), frequency_(440.0f), sample_rate_(48000) {
            std::cout << "Creating test audio processor (tone: "
                << (generate_tone ? "enabled" : "disabled") << ")" << std::endl;
        }

        void processAudio(const MultiChannelBuffer& inputs, MultiChannelBuffer& outputs, int num_samples) override {
            if (generate_tone_) {
                // Generate a simple sine wave tone
                for (int sample = 0; sample < num_samples; sample++) {
                    float value = 0.1f * std::sin(2.0f * 3.14159f * frequency_ * phase_ / sample_rate_);

                    for (size_t channel = 0; channel < outputs.size(); channel++) {
                        if (outputs[channel].size() > static_cast<size_t>(sample)) {
                            outputs[channel][sample] = value;
                        }
                    }

                    phase_ += 1.0f;
                    if (phase_ >= sample_rate_) {
                        phase_ = 0.0f;
                    }
                }
            }
            else {
                // Pass through (copy inputs to outputs)
                for (size_t channel = 0; channel < std::min(inputs.size(), outputs.size()); channel++) {
                    for (int sample = 0; sample < num_samples; sample++) {
                        if (inputs[channel].size() > static_cast<size_t>(sample) &&
                            outputs[channel].size() > static_cast<size_t>(sample)) {
                            outputs[channel][sample] = inputs[channel][sample];
                        }
                    }
                }
            }
        }

        void setupChanged(int sample_rate, int buffer_size) override {
            sample_rate_ = sample_rate;
            std::cout << "Audio setup changed: " << sample_rate << " Hz, " << buffer_size << " samples" << std::endl;
        }
    };

    // ====================================
    // Factory Functions - FIXED
    // ====================================

    std::unique_ptr<AudioInterface> createAudioInterface(HardwareType type) {
        std::cout << "Creating audio interface for: " << hardwareTypeToString(type) << std::endl;

#ifdef ENABLE_ASIO_SUPPORT
#ifdef _WIN32
        // Return ASIO interface for supported hardware types
        switch (type) {
        case HardwareType::UAD_APOLLO_X16:
        case HardwareType::UAD_APOLLO_X8:
        case HardwareType::ALLEN_HEATH_AVANTIS:
        case HardwareType::BEHRINGER_X32:
        case HardwareType::FOCUSRITE_SCARLETT:
        case HardwareType::RME_BABYFACE:
        case HardwareType::DIGICO_SD9:
        case HardwareType::YAMAHA_CL5:
        case HardwareType::GENERIC_ASIO:
            std::cout << "Creating ASIO interface for: " << hardwareTypeToString(type) << std::endl;
            // FIXED: Return as AudioInterface pointer (base class)
            return std::unique_ptr<AudioInterface>(new ASIOInterface());
        default:
            break;
        }
#endif
#endif

        // Fallback to stub interface for testing
        std::cout << "Creating stub interface for: " << hardwareTypeToString(type) << std::endl;
        return createStubInterface();
    }

    std::unique_ptr<AudioInterface> createStubInterface() {
        std::cout << "Creating stub audio interface..." << std::endl;
        // FIXED: Return as AudioInterface pointer (base class)  
        return std::unique_ptr<AudioInterface>(new StubAudioInterface());
    }

    std::unique_ptr<AudioProcessor> createTestProcessor(bool generate_tone) {
        return std::make_unique<TestAudioProcessor>(generate_tone);
    }

    // ====================================
    // Hardware Detection
    // ====================================

    std::vector<HardwareType> detectAvailableHardware() {
        std::vector<HardwareType> detected;

        std::cout << "Detecting available audio hardware..." << std::endl;

#ifdef ENABLE_ASIO_SUPPORT
        // Try to detect ASIO devices
        try {
            ASIOInterface temp_interface;
            if (temp_interface.initialize()) {
                auto drivers = temp_interface.getAvailableDrivers();
                for (const auto& driver : drivers) {
                    HardwareType type = temp_interface.detectHardwareType(driver);
                    detected.push_back(type);
                    std::cout << "  Found: " << driver << " (" << hardwareTypeToString(type) << ")" << std::endl;
                }
                temp_interface.shutdown();
            }
        }
        catch (const std::exception& e) {
            std::cout << "Error detecting ASIO hardware: " << e.what() << std::endl;
        }
#endif

        // Always include stub interface as fallback
        if (detected.empty()) {
            detected.push_back(HardwareType::GENERIC_ASIO);
            std::cout << "  Using stub interface (no hardware detected)" << std::endl;
        }

        std::cout << "Hardware detection complete (" << detected.size() << " devices)" << std::endl;
        return detected;
    }

    void printHardwareInfo(HardwareType type) {
        std::cout << "Hardware Info: " << hardwareTypeToString(type) << std::endl;

        auto interface = createAudioInterface(type);
        if (interface->initialize()) {
            std::cout << "  Name: " << interface->getName() << std::endl;
            std::cout << "  Input Channels: " << interface->getInputChannelCount() << std::endl;
            std::cout << "  Output Channels: " << interface->getOutputChannelCount() << std::endl;
            std::cout << "  Latency: " << interface->getCurrentLatency() << " ms" << std::endl;
            interface->shutdown();
        }
        else {
            std::cout << "  Failed to initialize" << std::endl;
        }
    }

    bool runBasicHardwareTest() {
        std::cout << "Running basic hardware test..." << std::endl;

        auto interface = createStubInterface();
        bool success = true;

        // Test initialization
        if (!interface->initialize()) {
            std::cout << "  ❌ Initialization failed" << std::endl;
            return false;
        }
        std::cout << "  ✅ Initialization successful" << std::endl;

        // Test processor creation and streaming
        auto processor = createTestProcessor(false);
        if (!interface->startStreaming(processor.get())) {
            std::cout << "  ❌ Streaming failed" << std::endl;
            success = false;
        }
        else {
            std::cout << "  ✅ Streaming successful" << std::endl;
            interface->stopStreaming();
        }

        // Test metrics
        auto metrics = interface->getMetrics();
        std::cout << "  Metrics - Latency: " << metrics.latency_ms << " ms, "
            << "CPU: " << metrics.cpu_usage_percent << "%, "
            << "Underruns: " << metrics.buffer_underruns << std::endl;

        interface->shutdown();
        std::cout << "Basic hardware test " << (success ? "PASSED" : "FAILED") << std::endl;
        return success;
    }

} // namespace Syntri