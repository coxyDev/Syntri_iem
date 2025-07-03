// src/hardware/asio_interface.cpp
// Fixed ASIO implementation with proper Windows headers and type handling
#include "syntri/asio_interface.h"
#include <iostream>
#include <chrono>
#include <algorithm>
#include <cstring>

// ASIO SDK integration with proper Windows headers - CRITICAL ORDER!
#ifdef ENABLE_ASIO_SUPPORT
#ifdef _WIN32

// 1. FIRST: Define all Windows macros before any Windows headers
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX  
#define NOMINMAX
#endif
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

// 2. SECOND: Include essential Windows headers for COM support
#include <windows.h>
#include <combaseapi.h>
#include <objbase.h>
#include <oleauto.h>
#include <guiddef.h>

// 3. THIRD: Define ASIO-specific macros before ASIO headers
#ifndef STRICT
#define STRICT 1
#endif

// 4. FOURTH: Now include ASIO SDK headers
#include "asio.h"
#include "asiodrivers.h"
#include "asiolist.h"

#endif // _WIN32
#endif // ENABLE_ASIO_SUPPORT

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

        // Initialize metrics
        metrics_.latency_ms = 0.0;
        metrics_.cpu_usage_percent = 0.0;
        metrics_.buffer_underruns = 0;

#ifdef ENABLE_ASIO_SUPPORT
        // Initialize ASIO drivers manager
        try {
            asio_drivers_ = std::make_unique<AsioDrivers>();
            std::cout << "ASIO drivers manager initialized" << std::endl;
        }
        catch (const std::exception& e) {
            std::cout << "Warning: Failed to initialize ASIO drivers: " << e.what() << std::endl;
        }
#endif
    }

    ASIOInterface::~ASIOInterface() {
        std::cout << "Destroying ASIO interface..." << std::endl;
        if (streaming_) {
            stopStreaming();
        }
        if (driver_loaded_) {
            unloadDriver();
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
            std::cout << "ASIO interface already initialized" << std::endl;
            return true;
        }

        sample_rate_ = sample_rate;
        buffer_size_ = buffer_size;

#ifdef ENABLE_ASIO_SUPPORT
        // Try to find and load the first available driver
        std::vector<std::string> drivers = getAvailableDrivers();
        if (drivers.empty()) {
            std::cout << "No ASIO drivers found - falling back to stub mode" << std::endl;
            detected_type_ = HardwareType::UNKNOWN;
            initialized_ = true;
            return true;
        }

        // Try to load the first driver
        if (loadDriver(drivers[0])) {
            std::cout << "ASIO driver loaded: " << drivers[0] << std::endl;
            detected_type_ = detectHardwareType(drivers[0]);
        }
        else {
            std::cout << "Failed to load ASIO driver - falling back to stub mode" << std::endl;
            detected_type_ = HardwareType::UNKNOWN;
        }
#else
        std::cout << "ASIO support not compiled in - using stub mode" << std::endl;
        detected_type_ = HardwareType::UNKNOWN;
#endif

        initialized_ = true;
        std::cout << "ASIO interface initialized (Type: " << hardwareTypeToString(detected_type_) << ")" << std::endl;
        return true;
    }

    void ASIOInterface::shutdown() {
        std::cout << "Shutting down ASIO interface..." << std::endl;
        if (streaming_) {
            stopStreaming();
        }
        if (driver_loaded_) {
            unloadDriver();
        }
        initialized_ = false;
        std::cout << "ASIO interface shut down" << std::endl;
    }

    bool ASIOInterface::isInitialized() const {
        return initialized_;
    }

    HardwareType ASIOInterface::getType() const {
        return detected_type_;
    }

    std::string ASIOInterface::getName() const {
        if (driver_loaded_ && !current_driver_name_.empty()) {
            return "ASIO: " + current_driver_name_;
        }
        return "ASIO Interface (Stub Mode)";
    }

    int ASIOInterface::getInputChannelCount() const {
        return input_channels_ > 0 ? input_channels_ : 2;  // Default to stereo
    }

    int ASIOInterface::getOutputChannelCount() const {
        return output_channels_ > 0 ? output_channels_ : 2;  // Default to stereo  
    }

    double ASIOInterface::getCurrentLatency() const {
        if (sample_rate_ > 0 && buffer_size_ > 0) {
            double latency = (static_cast<double>(buffer_size_) / static_cast<double>(sample_rate_)) * 1000.0;
            return latency + static_cast<double>(input_latency_ + output_latency_) / static_cast<double>(sample_rate_) * 1000.0;
        }
        return 0.0;
    }

    bool ASIOInterface::startStreaming(AudioProcessor* processor) {
        std::cout << "Starting ASIO streaming..." << std::endl;

        if (!initialized_) {
            std::cout << "ASIO interface not initialized" << std::endl;
            return false;
        }

        if (streaming_) {
            std::cout << "ASIO already streaming" << std::endl;
            return true;
        }

        processor_ = processor;

#ifdef ENABLE_ASIO_SUPPORT
        if (driver_loaded_) {
            // Real ASIO streaming
            if (setupASIOCallbacks() && createASIOBuffers()) {
                // Set sample rate using double (ASIOSampleRate is typedef double)
                double asio_rate = static_cast<double>(sample_rate_);
                if (ASIOSetSampleRate(asio_rate) == ASE_OK) {
                    if (ASIOStart() == ASE_OK) {
                        streaming_ = true;
                        std::cout << "ASIO streaming started with real driver" << std::endl;
                        return true;
                    }
                }
            }
            std::cout << "Failed to start real ASIO streaming - using simulation" << std::endl;
        }
#endif

        // Simulated streaming for stub mode
        streaming_ = true;
        if (processor_) {
            processor_->setupChanged(sample_rate_, buffer_size_);
        }
        std::cout << "ASIO streaming started (simulation mode)" << std::endl;
        return true;
    }

    void ASIOInterface::stopStreaming() {
        std::cout << "Stopping ASIO streaming..." << std::endl;

        if (!streaming_) {
            return;
        }

#ifdef ENABLE_ASIO_SUPPORT
        if (driver_loaded_) {
            ASIOStop();
            disposeASIOBuffers();
        }
#endif

        streaming_ = false;
        processor_ = nullptr;
        std::cout << "ASIO streaming stopped" << std::endl;
    }

    bool ASIOInterface::isStreaming() const {
        return streaming_;
    }

    SimpleMetrics ASIOInterface::getMetrics() const {
        // Update metrics
        SimpleMetrics current_metrics = metrics_;
        current_metrics.latency_ms = getCurrentLatency();
        current_metrics.buffer_underruns = 0; // Not tracking underruns yet
        return current_metrics;
    }

    std::vector<std::string> ASIOInterface::getAvailableDrivers() const {
        std::vector<std::string> drivers;

#ifdef ENABLE_ASIO_SUPPORT
        if (asio_drivers_) {
            // Correct ASIO API - getDriverNames expects char** not char[32][32]
            char* driver_names[32];  // Array of pointers
            for (int i = 0; i < 32; i++) {
                driver_names[i] = new char[32];  // Allocate space for each name
            }

            long num_drivers = asio_drivers_->getDriverNames(driver_names, 32);

            for (long i = 0; i < num_drivers; i++) {
                drivers.emplace_back(driver_names[i]);
            }

            // Clean up allocated memory
            for (int i = 0; i < 32; i++) {
                delete[] driver_names[i];
            }
        }
#endif

        return drivers;
    }

    bool ASIOInterface::loadDriver(const std::string& driver_name) {
#ifdef ENABLE_ASIO_SUPPORT
        if (!asio_drivers_) {
            return false;
        }

        if (driver_loaded_) {
            unloadDriver();
        }

        if (asio_drivers_->loadDriver(const_cast<char*>(driver_name.c_str()))) {
            // Initialize the driver
            ASIODriverInfo driver_info = {};
            driver_info.asioVersion = 2;  // Use ASIO version 2
            driver_info.sysRef = GetDesktopWindow();  // Windows handle

            if (ASIOInit(&driver_info) == ASE_OK) {
                current_driver_name_ = driver_name;
                driver_loaded_ = true;

                // Get channel counts and latencies
                long input_ch, output_ch;
                if (ASIOGetChannels(&input_ch, &output_ch) == ASE_OK) {
                    input_channels_ = static_cast<int>(input_ch);
                    output_channels_ = static_cast<int>(output_ch);
                }

                long input_lat, output_lat;
                if (ASIOGetLatencies(&input_lat, &output_lat) == ASE_OK) {
                    input_latency_ = static_cast<int>(input_lat);
                    output_latency_ = static_cast<int>(output_lat);
                }

                std::cout << "ASIO driver loaded: " << driver_name
                    << " (In: " << input_channels_ << ", Out: " << output_channels_ << ")" << std::endl;
                return true;
            }
        }
#endif
        return false;
    }

    void ASIOInterface::unloadDriver() {
#ifdef ENABLE_ASIO_SUPPORT
        if (driver_loaded_) {
            ASIOExit();
            if (asio_drivers_) {
                asio_drivers_->removeCurrentDriver();
            }
            driver_loaded_ = false;
            current_driver_name_.clear();
            std::cout << "ASIO driver unloaded" << std::endl;
        }
#endif
    }

    HardwareType ASIOInterface::detectHardwareType(const std::string& driver_name) const {
        // Convert driver name to lowercase for comparison
        std::string lower_name = driver_name;
        std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);

        if (lower_name.find("apollo") != std::string::npos) {
            if (lower_name.find("x16") != std::string::npos) return HardwareType::UAD_APOLLO_X16;
            if (lower_name.find("x8") != std::string::npos) return HardwareType::UAD_APOLLO_X8;
            return HardwareType::UAD_APOLLO_X8; // Default Apollo
        }

        if (lower_name.find("avantis") != std::string::npos) return HardwareType::ALLEN_HEATH_AVANTIS;
        if (lower_name.find("x32") != std::string::npos) return HardwareType::BEHRINGER_X32;
        if (lower_name.find("scarlett") != std::string::npos) return HardwareType::FOCUSRITE_SCARLETT;
        if (lower_name.find("babyface") != std::string::npos) return HardwareType::RME_BABYFACE;
        if (lower_name.find("digico") != std::string::npos || lower_name.find("sd9") != std::string::npos) return HardwareType::DIGICO_SD9;
        if (lower_name.find("yamaha") != std::string::npos || lower_name.find("cl5") != std::string::npos) return HardwareType::YAMAHA_CL5;

        return HardwareType::GENERIC_ASIO;
    }

    // Private implementation methods
    bool ASIOInterface::setupASIOCallbacks() {
#ifdef ENABLE_ASIO_SUPPORT
        callbacks_ = std::make_unique<ASIOCallbacks>();
        callbacks_->bufferSwitch = &ASIOInterface::bufferSwitch;
        callbacks_->sampleRateDidChange = &ASIOInterface::sampleRateChanged;
        callbacks_->asioMessage = &ASIOInterface::asioMessage;
        callbacks_->bufferSwitchTimeInfo = &ASIOInterface::bufferSwitchTimeInfo;
        return true;
#else
        return false;
#endif
    }

    bool ASIOInterface::createASIOBuffers() {
#ifdef ENABLE_ASIO_SUPPORT
        if (!driver_loaded_) return false;

        // Setup buffer info for stereo I/O
        buffer_infos_.clear();
        buffer_infos_.resize(4); // 2 in + 2 out

        // Input channels
        buffer_infos_[0].isInput = ASIOTrue;
        buffer_infos_[0].channelNum = 0;
        buffer_infos_[1].isInput = ASIOTrue;
        buffer_infos_[1].channelNum = 1;

        // Output channels  
        buffer_infos_[2].isInput = ASIOFalse;
        buffer_infos_[2].channelNum = 0;
        buffer_infos_[3].isInput = ASIOFalse;
        buffer_infos_[3].channelNum = 1;

        return ASIOCreateBuffers(buffer_infos_.data(), 4, buffer_size_, callbacks_.get()) == ASE_OK;
#else
        return false;
#endif
    }

    void ASIOInterface::disposeASIOBuffers() {
#ifdef ENABLE_ASIO_SUPPORT
        if (!buffer_infos_.empty()) {
            ASIODisposeBuffers();
            buffer_infos_.clear();
        }
#endif
    }

    // Static ASIO callback implementations
#ifdef ENABLE_ASIO_SUPPORT
    void ASIOInterface::bufferSwitch(long doubleBufferIndex, ASIOBool directProcess) {
        if (g_asio_instance && g_asio_instance->processor_) {
            // Simple buffer processing - in a real implementation, you'd process the actual ASIO buffers
            MultiChannelBuffer inputs(2, std::vector<float>(g_asio_instance->buffer_size_, 0.0f));
            MultiChannelBuffer outputs(2, std::vector<float>(g_asio_instance->buffer_size_, 0.0f));

            g_asio_instance->processor_->processAudio(inputs, outputs, g_asio_instance->buffer_size_);
            g_asio_instance->callback_count_++;
        }
    }

    void ASIOInterface::sampleRateChanged(double sRate) {
        // Handle sample rate changes
        if (g_asio_instance) {
            g_asio_instance->sample_rate_ = static_cast<int>(sRate);
        }
    }

    long ASIOInterface::asioMessage(long selector, long value, void* message, double* opt) {
        // Handle ASIO messages
        switch (selector) {
        case kAsioSelectorSupported:
            return 1L;
        case kAsioEngineVersion:
            return 2L;
        default:
            return 0L;
        }
    }

    ASIOTime* ASIOInterface::bufferSwitchTimeInfo(ASIOTime* params, long doubleBufferIndex, ASIOBool directProcess) {
        // Time-info version of buffer switch
        bufferSwitch(doubleBufferIndex, directProcess);
        return params;
    }
#endif

} // namespace Syntri