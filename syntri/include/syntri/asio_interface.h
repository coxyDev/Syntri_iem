// include/syntri/asio_interface.h
// Fixed ASIO interface with proper Windows headers and clean inheritance
#pragma once

#include "syntri/audio_interface.h"  // Include the main interface first
#include <memory>
#include <vector>
#include <string>
#include <chrono>

// Forward declarations for ASIO types to avoid header pollution
#ifdef ENABLE_ASIO_SUPPORT
#ifdef _WIN32
// Forward declare ASIO types
struct ASIODriverInfo;
struct ASIOChannelInfo;
struct ASIOBufferInfo;
struct ASIOCallbacks;
class AsioDrivers;

// ASIO types are defined in the ASIO headers - don't redefine them
#endif
#endif

namespace Syntri {

    // ASIO-specific interface that properly inherits from AudioInterface
    class ASIOInterface : public AudioInterface {
    private:
        // Core state
        bool initialized_;
        bool streaming_;
        bool driver_loaded_;
        std::string current_driver_name_;

        // Audio configuration  
        int sample_rate_;
        int buffer_size_;
        int input_channels_;
        int output_channels_;
        int input_latency_;
        int output_latency_;

        // ASIO-specific members
#ifdef ENABLE_ASIO_SUPPORT
        std::unique_ptr<AsioDrivers> asio_drivers_;
        std::vector<ASIOBufferInfo> buffer_infos_;
        std::unique_ptr<ASIOCallbacks> callbacks_;
#endif

        // Processing
        AudioProcessor* processor_;
        std::chrono::high_resolution_clock::time_point last_callback_time_;

        // Performance tracking
        int callback_count_;
        SimpleMetrics metrics_;
        HardwareType detected_type_;

    public:
        ASIOInterface();
        virtual ~ASIOInterface() override;

        // AudioInterface implementation
        virtual bool initialize(int sample_rate = SAMPLE_RATE_96K, int buffer_size = BUFFER_SIZE_ULTRA_LOW) override;
        virtual void shutdown() override;
        virtual bool isInitialized() const override;

        virtual HardwareType getType() const override;
        virtual std::string getName() const override;
        virtual int getInputChannelCount() const override;
        virtual int getOutputChannelCount() const override;
        virtual double getCurrentLatency() const override;

        virtual bool startStreaming(AudioProcessor* processor) override;
        virtual void stopStreaming() override;
        virtual bool isStreaming() const override;

        virtual SimpleMetrics getMetrics() const override;

        // ASIO-specific methods
        std::vector<std::string> getAvailableDrivers() const;
        bool loadDriver(const std::string& driver_name);
        void unloadDriver();
        HardwareType detectHardwareType(const std::string& driver_name) const;

    private:
        // Internal ASIO methods
        bool setupASIOCallbacks();
        bool createASIOBuffers();
        void disposeASIOBuffers();

        // Static callback methods (required by ASIO C API)
#ifdef ENABLE_ASIO_SUPPORT
        static void bufferSwitch(long doubleBufferIndex, bool directProcess);
        static void sampleRateChanged(ASIOSampleRate sRate);
        static long asioMessage(long selector, long value, void* message, double* opt);
        static auto bufferSwitchTimeInfo(void* params, long doubleBufferIndex, bool directProcess) -> void*;
#endif
    };

} // namespace Syntri