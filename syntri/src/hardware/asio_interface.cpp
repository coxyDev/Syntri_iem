// src/hardware/asio_interface.cpp
// Clean ASIO implementation that builds on your working foundation
#include "syntri/asio_interface.h"
#include <iostream>
#include <chrono>
#include <algorithm>
#include <cstring>

// ASIO SDK integration - only when available
#ifdef ENABLE_ASIO_SUPPORT
#ifdef _WIN32
#include <windows.h>

// Minimal ASIO declarations to avoid header issues
extern "C" {
    typedef long ASIOError;
    typedef double ASIOSampleRate;

#define ASE_OK 0
#define ASE_NotPresent -1000

    // ASIO function declarations
    ASIOError ASIOInit(void* driverInfo);
    void ASIOExit();
    ASIOError ASIOStart();
    ASIOError ASIOStop();
    ASIOError ASIOGetChannels(long* numInputChannels, long* numOutputChannels);
    ASIOError ASIOGetBufferSize(long* minSize, long* maxSize, long* preferredSize, long* granularity);
    ASIOError ASIOSetSampleRate(ASIOSampleRate sampleRate);
}
#endif
#endif

namespace Syntri {

    // Static instance pointer for ASIO callbacks
    static ASIOInterface* g_asio_instance = nullptr;

    ASIOInterface::ASIOInterface() {
        std::cout << "Creating ASIO interface..." << std::endl;
        g_asio_instance = this;
        last_callback_time_ = std::chrono::high_resolution_clock::now();
    }

    ASIOInterface::~ASIOInterface() {
        std::cout << "Destroying ASIO interface..." << std::endl;
        if (initialized_) {
            shutdown();
        }
        g_asio_instance = nullptr;
    }

    bool ASIOInterface::initialize(int sample_rate, int buffer_size) {
        std::cout << "Initializing ASIO interface..." << std::endl;

        sample_rate_ = sample_rate;
        buffer_size_ = buffer_size;

#ifdef ENABLE_ASIO_SUPPORT
#ifdef _WIN32
        if (!initializeASIO()) {
            std::cout << "   ASIO initialization failed - using simulation mode" << std::endl;
            // Don't return false - fall back to simulation mode
        }
        else {
            std::cout << "   Real ASIO hardware initialized successfully" << std::endl;
        }
#else
        std::cout << "   ASIO only supported on Windows - using simulation mode" << std::endl;
#endif
#else
        std::cout << "   ASIO support not compiled in - using simulation mode" << std::endl;
#endif

        // Always succeed - either with real ASIO or simulation
        initialized_ = true;
        return true;
    }

    void ASIOInterface::shutdown() {
        if (!initialized_) return;

        std::cout << "Shutting down ASIO interface..." << std::endl;

        if (streaming_) {
            stopStreaming();
        }

#ifdef ENABLE_ASIO_SUPPORT
#ifdef _WIN32
        cleanupASIO();
#endif
#endif

        initialized_ = false;
        std::cout << "   ASIO interface shutdown complete" << std::endl;
    }

    bool ASIOInterface::initializeASIO() {
#ifdef ENABLE_ASIO_SUPPORT
#ifdef _WIN32
        try {
            std::cout << "   Attempting real ASIO driver initialization..." << std::endl;

            // Try to initialize ASIO
            if (ASIOInit(nullptr) != ASE_OK) {
                std::cout << "   No ASIO drivers found" << std::endl;
                return false;
            }

            std::cout << "   ASIO drivers found and initialized" << std::endl;

            // Get channel information
            long input_ch = 0, output_ch = 0;
            if (ASIOGetChannels(&input_ch, &output_ch) != ASE_OK) {
                std::cout << "   Failed to get ASIO channel information" << std::endl;
                ASIOExit();
                return false;
            }

            input_channels_ = static_cast<int>(input_ch);
            output_channels_ = static_cast<int>(output_ch);

            std::cout << "   Found " << input_channels_ << " input channels, "
                << output_channels_ << " output channels" << std::endl;

            // Detect hardware type
            detected_type_ = detectHardwareType("Generic ASIO");

            // Set sample rate
            ASIOSampleRate asio_sample_rate = static_cast<ASIOSampleRate>(sample_rate_);
            if (ASIOSetSampleRate(asio_sample_rate) != ASE_OK) {
                std::cout << "   Warning: Could not set preferred sample rate" << std::endl;
            }

            // Get buffer size information
            long min_size, max_size, preferred_size, granularity;
            if (ASIOGetBufferSize(&min_size, &max_size, &preferred_size, &granularity) != ASE_OK) {
                std::cout << "   Failed to get ASIO buffer size information" << std::endl;
                ASIOExit();
                return false;
            }

            // Use preferred buffer size for now
            buffer_size_ = static_cast<int>(preferred_size);
            std::cout << "   Using buffer size: " << buffer_size_ << " samples" << std::endl;

            // Initialize audio buffers
            input_buffers_.resize(input_channels_);
            output_buffers_.resize(output_channels_);
            for (auto& buffer : input_buffers_) {
                buffer.resize(buffer_size_);
            }
            for (auto& buffer : output_buffers_) {
                buffer.resize(buffer_size_);
            }

            return true;

        }
        catch (const std::exception& e) {
            std::cout << "   ASIO initialization exception: " << e.what() << std::endl;
            return false;
        }
        catch (...) {
            std::cout << "   Unknown ASIO initialization error" << std::endl;
            return false;
        }
#endif
#endif
        return false;
    }

    void ASIOInterface::cleanupASIO() {
#ifdef ENABLE_ASIO_SUPPORT
#ifdef _WIN32
        try {
            ASIOExit();
            std::cout << "   ASIO cleanup completed" << std::endl;
        }
        catch (...) {
            std::cout << "   ASIO cleanup had issues (continuing anyway)" << std::endl;
        }
#endif
#endif
    }

    HardwareType ASIOInterface::detectHardwareType(const std::string& driver_name) {
        // Simple detection based on driver name or channel count
        std::string name_lower = driver_name;
        std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(), ::tolower);

        if (name_lower.find("apollo") != std::string::npos) {
            return (input_channels_ >= 16) ? HardwareType::UAD_APOLLO_X16 : HardwareType::GENERIC_ASIO;
        }
        else if (name_lower.find("avantis") != std::string::npos) {
            return HardwareType::ALLEN_HEATH_AVANTIS;
        }
        else if (name_lower.find("x32") != std::string::npos) {
            return HardwareType::BEHRINGER_X32;
        }

        return HardwareType::GENERIC_ASIO;
    }

    // AudioInterface implementation
    bool ASIOInterface::isInitialized() const {
        return initialized_;
    }

    HardwareType ASIOInterface::getType() const {
        return detected_type_;
    }

    std::string ASIOInterface::getName() const {
        return "ASIO Audio Interface";
    }

    int ASIOInterface::getInputChannelCount() const {
        return input_channels_ > 0 ? input_channels_ : 8;  // Default to 8 if not detected
    }

    int ASIOInterface::getOutputChannelCount() const {
        return output_channels_ > 0 ? output_channels_ : 8;  // Default to 8 if not detected
    }

    double ASIOInterface::getCurrentLatency() const {
        // Calculate latency based on buffer size and sample rate
        return (static_cast<double>(buffer_size_) / sample_rate_) * 1000.0 + 0.5; // +0.5ms for processing
    }

    bool ASIOInterface::startStreaming(AudioProcessor* processor) {
        if (!initialized_ || !processor) return false;

        processor_ = processor;

#ifdef ENABLE_ASIO_SUPPORT
#ifdef _WIN32
        try {
            if (ASIOStart() == ASE_OK) {
                streaming_ = true;
                std::cout << "   Real ASIO streaming started" << std::endl;
                return true;
            }
            else {
                std::cout << "   ASIO start failed - using simulation mode" << std::endl;
            }
        }
        catch (...) {
            std::cout << "   Exception starting ASIO - using simulation mode" << std::endl;
        }
#endif
#endif

        // Simulation mode - always succeeds
        streaming_ = true;
        std::cout << "   ASIO simulation streaming started" << std::endl;
        return true;
    }

    void ASIOInterface::stopStreaming() {
        if (!streaming_) return;

#ifdef ENABLE_ASIO_SUPPORT
#ifdef _WIN32
        if (streaming_) {
            try {
                ASIOStop();
                std::cout << "   Real ASIO streaming stopped" << std::endl;
            }
            catch (...) {
                std::cout << "   ASIO stop had issues" << std::endl;
            }
        }
#endif
#endif

        streaming_ = false;
        processor_ = nullptr;
        std::cout << "   ASIO streaming stopped" << std::endl;
    }

    bool ASIOInterface::isStreaming() const {
        return streaming_;
    }

    SimpleMetrics ASIOInterface::getMetrics() const {
        return metrics_;
    }

    // Static ASIO callbacks (would be connected to real ASIO driver)
    void ASIOInterface::bufferSwitch(long doubleBufferIndex, long directProcess) {
        if (g_asio_instance) {
            g_asio_instance->processAudioCallback(doubleBufferIndex);
        }
    }

    void ASIOInterface::sampleRateDidChange(double sRate) {
        std::cout << "   ASIO sample rate changed to: " << sRate << " Hz" << std::endl;
    }

    long ASIOInterface::asioMessage(long selector, long value, void* message, double* opt) {
        // Handle ASIO messages
        return 0;
    }

    void ASIOInterface::processAudioCallback(long bufferIndex) {
        if (!processor_ || !streaming_) return;

        auto start_time = std::chrono::high_resolution_clock::now();

        // Update callback count
        callback_count_++;

        // Process audio through our processor
        if (processor_) {
            processor_->processAudio(input_buffers_, output_buffers_, buffer_size_);
        }

        // Update performance metrics
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        metrics_.latency_ms = duration.count() / 1000.0;

        last_callback_time_ = end_time;
    }

    // Factory function
    std::unique_ptr<ASIOInterface> createASIOInterface() {
        return std::make_unique<ASIOInterface>();
    }

} // namespace Syntri