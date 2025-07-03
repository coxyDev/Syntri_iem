// src/hardware/asio_interface.cpp
// Corrected ASIO implementation using your existing type structure
#include "syntri/asio_interface.h"
#include <iostream>
#include <chrono>
#include <algorithm>
#include <cstring>

// ASIO SDK integration with proper Windows headers
#ifdef ENABLE_ASIO_SUPPORT
#ifdef _WIN32
// Include Windows headers first for COM support
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <combaseapi.h>
#include <objbase.h>

// Now include ASIO SDK headers
#include "asio.h"
#include "asiodrivers.h"
#include "asiolist.h"
#endif
#endif

namespace Syntri {

    // Global instance for ASIO callbacks (required for C callback interface)
    static ASIOInterface* g_asio_instance = nullptr;

    ASIOInterface::ASIOInterface()
        : initialized_(false)
        , streaming_(false)
        , driver_loaded_(false)
        , current_driver_name_("")
        , sample_rate_(SAMPLE_RATE_96K)
        , buffer_size_(BUFFER_SIZE_ULTRA_LOW)
        , input_channels_(0)
        , output_channels_(0)
        , input_latency_(0)
        , output_latency_(0)
        , processor_(nullptr)
        , callback_count_(0)
        , detected_type_(HardwareType::UNKNOWN)
    {
        std::cout << "Creating ASIO interface..." << std::endl;
        g_asio_instance = this;
        last_callback_time_ = std::chrono::high_resolution_clock::now();

#ifdef ENABLE_ASIO_SUPPORT
        // Initialize ASIO drivers manager
        asio_drivers_ = std::make_unique<AsioDrivers>();
        std::cout << "ASIO drivers manager initialized" << std::endl;
#else
        std::cout << "ASIO support not compiled - using stub interface" << std::endl;
#endif
    }

    ASIOInterface::~ASIOInterface() {
        std::cout << "Destroying ASIO interface..." << std::endl;
        if (streaming_) {
            stopStreaming();
        }
        if (initialized_) {
            shutdown();
        }
        g_asio_instance = nullptr;
    }

    bool ASIOInterface::initialize(int sample_rate, int buffer_size) {
        std::cout << "Initializing ASIO interface (SR: " << sample_rate
            << " Hz, Buffer: " << buffer_size << ")" << std::endl;

        if (initialized_) {
            std::cout << "Already initialized - shutting down first" << std::endl;
            shutdown();
        }

        sample_rate_ = sample_rate;
        buffer_size_ = buffer_size;

#ifdef ENABLE_ASIO_SUPPORT
        // Try to find and load a suitable ASIO driver
        auto drivers = getAvailableDrivers();
        if (drivers.empty()) {
            std::cout << "No ASIO drivers found" << std::endl;
            return false;
        }

        std::cout << "Found " << drivers.size() << " ASIO driver(s):" << std::endl;
        for (const auto& driver : drivers) {
            std::cout << "  - " << driver << std::endl;
        }

        // Try to load the first available driver
        if (!loadDriver(drivers[0])) {
            std::cout << "Failed to load ASIO driver: " << drivers[0] << std::endl;
            return false;
        }

        // Initialize ASIO system
        if (!initializeASIO()) {
            std::cout << "Failed to initialize ASIO system" << std::endl;
            unloadDriver();
            return false;
        }

        initialized_ = true;
        std::cout << "ASIO interface initialized successfully" << std::endl;
        return true;

#else
        // Stub implementation when ASIO not available
        initialized_ = true;
        detected_type_ = HardwareType::GENERIC_ASIO;
        input_channels_ = 2;
        output_channels_ = 2;
        std::cout << "ASIO stub interface initialized (no real hardware)" << std::endl;
        return true;
#endif
    }

    void ASIOInterface::shutdown() {
        std::cout << "Shutting down ASIO interface..." << std::endl;

        if (streaming_) {
            stopStreaming();
        }

#ifdef ENABLE_ASIO_SUPPORT
        cleanupASIO();
        unloadDriver();
#endif

        initialized_ = false;
        std::cout << "ASIO interface shut down" << std::endl;
    }

    bool ASIOInterface::startStreaming(AudioProcessor* processor) {
        if (!initialized_) {
            std::cout << "Cannot start streaming - interface not initialized" << std::endl;
            return false;
        }

        if (streaming_) {
            std::cout << "Already streaming" << std::endl;
            return true;
        }

        processor_ = processor;
        callback_count_ = 0;

        std::cout << "Starting ASIO streaming..." << std::endl;

#ifdef ENABLE_ASIO_SUPPORT
        if (driver_loaded_) {
            ASIOError result = ASIOStart();
            if (result == 0) { // ASE_OK
                streaming_ = true;
                std::cout << "ASIO streaming started successfully" << std::endl;
                return true;
            }
            else {
                std::cout << "ASIOStart failed with error: " << result << std::endl;
                return false;
            }
        }
#endif

        // Stub implementation
        streaming_ = true;
        std::cout << "ASIO stub streaming started (simulated)" << std::endl;
        return true;
    }

    void ASIOInterface::stopStreaming() {
        if (!streaming_) {
            return;
        }

        std::cout << "Stopping ASIO streaming..." << std::endl;

#ifdef ENABLE_ASIO_SUPPORT
        if (driver_loaded_) {
            ASIOError result = ASIOStop();
            if (result != 0) { // Not ASE_OK
                std::cout << "ASIOStop returned error: " << result << std::endl;
            }
        }
#endif

        streaming_ = false;
        processor_ = nullptr;

        std::cout << "ASIO streaming stopped (processed " << callback_count_ << " callbacks)" << std::endl;
    }

    // AudioInterface implementation - using your exact method signatures
    bool ASIOInterface::isInitialized() const {
        return initialized_;
    }

    HardwareType ASIOInterface::getType() const {
        return detected_type_;
    }

    std::string ASIOInterface::getName() const {
        if (!current_driver_name_.empty()) {
            return current_driver_name_;
        }
        return "ASIO Audio Interface";
    }

    int ASIOInterface::getInputChannelCount() const {
        return input_channels_ > 0 ? input_channels_ : 2;  // Default to stereo
    }

    int ASIOInterface::getOutputChannelCount() const {
        return output_channels_ > 0 ? output_channels_ : 2;  // Default to stereo
    }

    double ASIOInterface::getCurrentLatency() const {
        if (buffer_size_ > 0 && sample_rate_ > 0) {
            return (static_cast<double>(buffer_size_) / static_cast<double>(sample_rate_)) * 1000.0;
        }
        return 10.0; // Default 10ms
    }

    bool ASIOInterface::isStreaming() const {
        return streaming_;
    }

    SimpleMetrics ASIOInterface::getMetrics() const {
        SimpleMetrics metrics;
        metrics.latency_ms = getCurrentLatency();
        metrics.cpu_usage_percent = 5.0; // Placeholder
        metrics.buffer_underruns = 0; // Not implemented yet
        return metrics;
    }

    std::vector<HardwareType> ASIOInterface::detectHardwareTypes() const {
        std::vector<HardwareType> hardware_types;

        std::cout << "Detecting ASIO hardware types..." << std::endl;

#ifdef ENABLE_ASIO_SUPPORT
        auto drivers = getAvailableDrivers();
        for (const auto& driver_name : drivers) {
            HardwareType type = detectHardwareType(driver_name);
            hardware_types.push_back(type);
            std::cout << "  " << hardwareTypeToString(type) << " (" << driver_name << ")" << std::endl;
        }
#else
        // Stub implementation for testing
        hardware_types.push_back(HardwareType::GENERIC_ASIO);
        std::cout << "  Generic ASIO (stub for testing)" << std::endl;
#endif

        std::cout << "Found " << hardware_types.size() << " ASIO device type(s)" << std::endl;
        return hardware_types;
    }

    std::vector<std::string> ASIOInterface::getAvailableDrivers() const {
        std::vector<std::string> drivers;

#ifdef ENABLE_ASIO_SUPPORT
        if (asio_drivers_) {
            char* driver_names[32];  // Max 32 drivers as per ASIO spec
            long num_drivers = asio_drivers_->getDriverNames(driver_names, 32);

            for (long i = 0; i < num_drivers; i++) {
                drivers.push_back(std::string(driver_names[i]));
            }
        }
#endif

        return drivers;
    }

    bool ASIOInterface::loadDriver(const std::string& driver_name) {
        std::cout << "Loading ASIO driver: " << driver_name << std::endl;

        if (driver_loaded_) {
            unloadDriver();
        }

#ifdef ENABLE_ASIO_SUPPORT
        if (asio_drivers_) {
            // Note: loadDriver expects non-const char*, but doesn't modify it
            char* name = const_cast<char*>(driver_name.c_str());
            if (asio_drivers_->loadDriver(name)) {
                driver_loaded_ = true;
                current_driver_name_ = driver_name;
                detected_type_ = detectHardwareType(driver_name);
                std::cout << "ASIO driver loaded: " << driver_name << std::endl;
                return true;
            }
            else {
                std::cout << "Failed to load ASIO driver: " << driver_name << std::endl;
                return false;
            }
        }
#endif

        // Stub implementation
        driver_loaded_ = true;
        current_driver_name_ = driver_name;
        detected_type_ = detectHardwareType(driver_name);
        std::cout << "ASIO driver stub loaded: " << driver_name << std::endl;
        return true;
    }

    void ASIOInterface::unloadDriver() {
        if (!driver_loaded_) {
            return;
        }

        std::cout << "Unloading ASIO driver: " << current_driver_name_ << std::endl;

#ifdef ENABLE_ASIO_SUPPORT
        // ASIO drivers are automatically unloaded when AsioDrivers is destroyed
        // or when a new driver is loaded
#endif

        driver_loaded_ = false;
        current_driver_name_.clear();
        detected_type_ = HardwareType::UNKNOWN;
        std::cout << "ASIO driver unloaded" << std::endl;
    }

    bool ASIOInterface::initializeASIO() {
#ifdef ENABLE_ASIO_SUPPORT
        if (!driver_loaded_) {
            return false;
        }

        std::cout << "Initializing ASIO system..." << std::endl;

        // Initialize ASIO
        ASIODriverInfo driver_info = { 0 };
        driver_info.asioVersion = 2;  // We support ASIO 2.0
        driver_info.sysRef = GetForegroundWindow();  // Windows handle

        ASIOError result = ASIOInit(&driver_info);
        if (result != 0) { // Not ASE_OK
            std::cout << "ASIOInit failed: " << result << std::endl;
            return false;
        }

        std::cout << "  Driver: " << driver_info.name << std::endl;
        std::cout << "  Version: " << driver_info.driverVersion << std::endl;

        // Get channel counts
        long inputs, outputs;
        result = ASIOGetChannels(&inputs, &outputs);
        if (result != 0) {
            std::cout << "ASIOGetChannels failed: " << result << std::endl;
            ASIOExit();
            return false;
        }

        input_channels_ = static_cast<int>(inputs);
        output_channels_ = static_cast<int>(outputs);
        std::cout << "  Input channels: " << input_channels_ << std::endl;
        std::cout << "  Output channels: " << output_channels_ << std::endl;

        // Set sample rate
        result = ASIOSetSampleRate(static_cast<ASIOSampleRate>(sample_rate_));
        if (result != 0) {
            std::cout << "ASIOSetSampleRate failed: " << result << std::endl;
            ASIOExit();
            return false;
        }

        // Get latencies
        long input_latency, output_latency;
        result = ASIOGetLatencies(&input_latency, &output_latency);
        if (result == 0) {
            input_latency_ = static_cast<int>(input_latency);
            output_latency_ = static_cast<int>(output_latency);
            std::cout << "  Input latency: " << input_latency_ << " samples" << std::endl;
            std::cout << "  Output latency: " << output_latency_ << " samples" << std::endl;
        }

        // Initialize audio buffers using your MultiChannelBuffer type
        input_buffers_.resize(input_channels_);
        output_buffers_.resize(output_channels_);
        for (auto& buffer : input_buffers_) {
            buffer.resize(buffer_size_);
        }
        for (auto& buffer : output_buffers_) {
            buffer.resize(buffer_size_);
        }

        return true;
#else
        return true;  // Stub always succeeds
#endif
    }

    void ASIOInterface::cleanupASIO() {
#ifdef ENABLE_ASIO_SUPPORT
        if (driver_loaded_) {
            releaseBuffers();
            ASIOExit();
        }
#endif
    }

    bool ASIOInterface::configureBuffers() {
#ifdef ENABLE_ASIO_SUPPORT
        std::cout << "Configuring ASIO buffers..." << std::endl;

        // For now, we'll just verify buffer sizes are available
        long min_size, max_size, preferred_size, granularity;
        ASIOError result = ASIOGetBufferSize(&min_size, &max_size, &preferred_size, &granularity);
        if (result != 0) {
            std::cout << "ASIOGetBufferSize failed: " << result << std::endl;
            return false;
        }

        std::cout << "  Buffer sizes - Min: " << min_size
            << ", Max: " << max_size
            << ", Preferred: " << preferred_size << std::endl;

        // Use preferred size for now (full buffer setup would be more complex)
        buffer_size_ = static_cast<int>(preferred_size);

        return true;
#else
        return true;
#endif
    }

    void ASIOInterface::releaseBuffers() {
#ifdef ENABLE_ASIO_SUPPORT
        // Buffer disposal would happen here in full implementation
#endif
    }

    // ASIO Callback Functions (must be static for C interface)
    void ASIOInterface::bufferSwitch(long index, long processNow) {
        if (g_asio_instance) {
            g_asio_instance->handleBufferSwitch(index);
        }
    }

    void ASIOInterface::sampleRateChanged(double sRate) {
        std::cout << "ASIO sample rate changed to: " << sRate << " Hz" << std::endl;
    }

    long ASIOInterface::asioMessage(long selector, long value, void* message, double* opt) {
        // Handle ASIO messages here
        return 0;
    }

    long ASIOInterface::bufferSwitchTimeInfo(void* params, long index, long processNow) {
        bufferSwitch(index, processNow);
        return 0;
    }

    void ASIOInterface::handleBufferSwitch(long buffer_index) {
        callback_count_++;
        last_callback_time_ = std::chrono::high_resolution_clock::now();

        if (processor_) {
            processAudioCallback(buffer_size_);
        }
    }

    void ASIOInterface::processAudioCallback(int frame_count) {
        if (processor_) {
            // Call processor using your exact interface signature:
            // processAudio(const MultiChannelBuffer& inputs, MultiChannelBuffer& outputs, int num_samples)
            processor_->processAudio(input_buffers_, output_buffers_, frame_count);
        }
    }

    HardwareType ASIOInterface::detectHardwareType(const std::string& driver_name) const {
        // Basic pattern matching for known hardware types
        std::string lower_name = driver_name;
        std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);

        if (lower_name.find("uad") != std::string::npos ||
            lower_name.find("apollo") != std::string::npos) {
            // Determine Apollo model based on channel count or name
            if (lower_name.find("x16") != std::string::npos || input_channels_ >= 16) {
                return HardwareType::UAD_APOLLO_X16;
            }
            else {
                return HardwareType::UAD_APOLLO_X8;
            }
        }
        else if (lower_name.find("avantis") != std::string::npos ||
            lower_name.find("allen") != std::string::npos) {
            return HardwareType::ALLEN_HEATH_AVANTIS;
        }
        else if (lower_name.find("x32") != std::string::npos ||
            lower_name.find("behringer") != std::string::npos) {
            return HardwareType::BEHRINGER_X32;
        }
        else if (lower_name.find("rme") != std::string::npos) {
            return HardwareType::RME_BABYFACE;
        }
        else if (lower_name.find("focusrite") != std::string::npos) {
            return HardwareType::FOCUSRITE_SCARLETT;
        }
        else if (lower_name.find("digico") != std::string::npos) {
            return HardwareType::DIGICO_SD9;
        }
        else if (lower_name.find("yamaha") != std::string::npos ||
            lower_name.find("cl5") != std::string::npos) {
            return HardwareType::YAMAHA_CL5;
        }

        return HardwareType::GENERIC_ASIO;  // Default
    }

    std::string ASIOInterface::getDriverDescription(const std::string& driver_name) const {
        // Return description based on detected hardware type
        HardwareType type = detectHardwareType(driver_name);
        return hardwareTypeToString(type);
    }

} // namespace Syntri