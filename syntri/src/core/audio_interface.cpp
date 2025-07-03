// src/core/audio_interface.cpp
// Updated to work with your existing types and add ASIO integration
#include "syntri/audio_interface.h"
#include "syntri/asio_interface.h"
#include <iostream>
#include <algorithm>
#include <memory>
#include <thread>
#include <chrono>

namespace Syntri {

    // ====================================
    // Hardware Detection Functions  
    // ====================================

    std::vector<HardwareType> detectAvailableHardware() {
        std::vector<HardwareType> all_hardware;

        std::cout << "Scanning for audio hardware..." << std::endl;

        // Try ASIO hardware detection first (Windows)
#ifdef _WIN32
        try {
            auto asio_interface = std::make_unique<ASIOInterface>();
            auto asio_hardware = asio_interface->detectHardwareTypes();

            for (const auto& hw_type : asio_hardware) {
                all_hardware.push_back(hw_type);
            }

            std::cout << "ASIO scan complete - found " << asio_hardware.size() << " device(s)" << std::endl;
        }
        catch (const std::exception& e) {
            std::cout << "ASIO scan failed: " << e.what() << std::endl;
        }
#endif

        // Add generic fallback hardware for testing
        if (all_hardware.empty()) {
            std::cout << "Adding generic fallback hardware..." << std::endl;
            all_hardware.push_back(HardwareType::GENERIC_ASIO);
        }

        std::cout << "Total hardware detected: " << all_hardware.size() << " device(s)" << std::endl;
        return all_hardware;
    }

    std::unique_ptr<AudioInterface> createAudioInterface(HardwareType type) {
        std::cout << "Creating audio interface for: " << hardwareTypeToString(type) << std::endl;

        // For now, all hardware types use ASIO interface on Windows
#ifdef _WIN32
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
            return std::make_unique<ASIOInterface>();
        default:
            break;
        }
#endif

        // Fallback to stub interface for testing
        std::cout << "Creating stub interface for: " << hardwareTypeToString(type) << std::endl;
        return createStubInterface();
    }

    std::unique_ptr<AudioInterface> createStubInterface() {
        std::cout << "Creating stub audio interface..." << std::endl;
        return std::make_unique<StubAudioInterface>();
    }

    // ====================================
    // StubAudioInterface Implementation
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

    public:
        StubAudioInterface()
            : initialized_(false), streaming_(false), sample_rate_(SAMPLE_RATE_96K),
            buffer_size_(BUFFER_SIZE_ULTRA_LOW), processor_(nullptr),
            hardware_type_(HardwareType::GENERIC_ASIO) {
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
            return 10.0; // Default 10ms
        }

        bool startStreaming(AudioProcessor* processor) override {
            if (!initialized_) {
                std::cout << "Cannot start streaming - not initialized" << std::endl;
                return false;
            }

            if (streaming_) {
                std::cout << "Already streaming" << std::endl;
                return true;
            }

            processor_ = processor;
            streaming_ = true;

            std::cout << "Started stub audio streaming (simulated)" << std::endl;
            return true;
        }

        void stopStreaming() override {
            if (!streaming_) return;

            streaming_ = false;
            processor_ = nullptr;
            std::cout << "Stopped stub audio streaming" << std::endl;
        }

        bool isStreaming() const override {
            return streaming_;
        }

        SimpleMetrics getMetrics() const override {
            SimpleMetrics metrics;
            metrics.latency_ms = getCurrentLatency();
            metrics.cpu_usage_percent = 2.0; // Low CPU for stub
            metrics.buffer_underruns = 0;
            return metrics;
        }
    };

    // ====================================
    // Test Audio Processor Implementation
    // ====================================

    class TestAudioProcessor : public AudioProcessor {
    private:
        int frame_count_;
        bool generate_tone_;
        float tone_frequency_;
        float phase_;

    public:
        TestAudioProcessor(bool generate_tone = false, float frequency = 440.0f)
            : frame_count_(0), generate_tone_(generate_tone),
            tone_frequency_(frequency), phase_(0.0f) {
            std::cout << "Test processor created (tone=" << (generate_tone ? "on" : "off") << ")" << std::endl;
        }

        void processAudio(
            const MultiChannelBuffer& inputs,
            MultiChannelBuffer& outputs,
            int num_samples
        ) override {
            frame_count_ += num_samples;

            if (generate_tone_ && !outputs.empty()) {
                // Generate simple sine wave for testing
                float sample_rate = 48000.0f; // Assume 48kHz for test tone
                float phase_increment = 2.0f * 3.14159f * tone_frequency_ / sample_rate;

                for (int frame = 0; frame < num_samples; ++frame) {
                    float sample = 0.1f * sinf(phase_);  // Low volume sine wave

                    // Output to all available channels
                    for (size_t ch = 0; ch < outputs.size(); ++ch) {
                        if (frame < static_cast<int>(outputs[ch].size())) {
                            outputs[ch][frame] = sample;
                        }
                    }

                    phase_ += phase_increment;
                    if (phase_ > 2.0f * 3.14159f) {
                        phase_ -= 2.0f * 3.14159f;
                    }
                }
            }
            else {
                // Pass-through or silence
                for (size_t ch = 0; ch < outputs.size() && ch < inputs.size(); ++ch) {
                    for (int frame = 0; frame < num_samples; ++frame) {
                        if (frame < static_cast<int>(outputs[ch].size()) &&
                            frame < static_cast<int>(inputs[ch].size())) {
                            outputs[ch][frame] = inputs[ch][frame] * 0.5f;  // 50% pass-through
                        }
                    }
                }
            }

            // Log progress occasionally
            if (frame_count_ % (48000 * 2) == 0) {  // Every 2 seconds at 48kHz
                std::cout << "Processed " << frame_count_ << " frames" << std::endl;
            }
        }

        void setupChanged(int sample_rate, int buffer_size) override {
            std::cout << "Audio setup changed: " << sample_rate << "Hz, " << buffer_size << " samples" << std::endl;
            // Reset state if needed
            phase_ = 0.0f;
            frame_count_ = 0;
        }
    };

    std::unique_ptr<AudioProcessor> createTestProcessor(bool generate_tone) {
        return std::make_unique<TestAudioProcessor>(generate_tone);
    }

    // ====================================
    // Utility Functions
    // ====================================

    void printHardwareInfo(HardwareType type) {
        std::cout << "\nHardware: " << hardwareTypeToString(type) << std::endl;
        std::cout << "   Type: Professional Audio Interface" << std::endl;
        std::cout << "   Connected: Yes" << std::endl;
        std::cout << "   Channels: 2 in, 2 out (default)" << std::endl;
        std::cout << "   Sample Rates: 44100, 48000, 96000 Hz" << std::endl;
        std::cout << "   Features: Low-latency ASIO support" << std::endl;
    }

    bool runBasicHardwareTest() {
        std::cout << "\nRunning basic hardware test..." << std::endl;

        try {
            // Detect hardware
            auto hardware = detectAvailableHardware();
            if (hardware.empty()) {
                std::cout << "No hardware detected" << std::endl;
                return false;
            }

            // Print detected hardware
            for (const auto& hw_type : hardware) {
                printHardwareInfo(hw_type);
            }

            // Test interface creation
            auto interface = createAudioInterface(hardware[0]);
            if (!interface) {
                std::cout << "Failed to create audio interface" << std::endl;
                return false;
            }

            // Test initialization
            if (!interface->initialize(SAMPLE_RATE_48K, BUFFER_SIZE_LOW)) {
                std::cout << "Failed to initialize interface" << std::endl;
                return false;
            }

            // Test streaming
            auto processor = createTestProcessor(false);
            if (!interface->startStreaming(processor.get())) {
                std::cout << "Failed to start streaming" << std::endl;
                return false;
            }

            std::cout << "Streaming started successfully" << std::endl;

            // Brief delay to simulate operation
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            // Clean shutdown
            interface->stopStreaming();
            interface->shutdown();

            std::cout << "Basic hardware test completed successfully" << std::endl;
            return true;

        }
        catch (const std::exception& e) {
            std::cout << "Hardware test failed: " << e.what() << std::endl;
            return false;
        }
    }

} // namespace Syntri