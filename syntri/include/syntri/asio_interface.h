// Fixed include/syntri/asio_interface.h
#pragma once

#include "syntri/core_engine.h"
#include <memory>
#include <vector>
#include <atomic>
#include <chrono>
#include <string>

// Forward declare ASIO types to avoid including problematic headers in the interface
typedef void* ASIODriverInfo;
typedef void* ASIOBufferInfo;
typedef void* ASIOCallbacks;
typedef void* ASIOChannelInfo;
typedef long ASIOError;
typedef double ASIOSampleRate;

namespace Syntri {

    // ASIO Interface - Real hardware communication when ASIO SDK is available
    class ASIOInterface : public AudioInterface {
    private:
        // Core state
        bool initialized_ = false;
        bool streaming_ = false;
        HardwareType detected_type_ = HardwareType::GENERIC_ASIO;
        PerformanceMetrics metrics_;
        AudioProcessor* processor_ = nullptr;

        // Audio configuration
        int sample_rate_ = SAMPLE_RATE_96K;
        int buffer_size_ = BUFFER_SIZE_ULTRA_LOW;
        int input_channels_ = 0;
        int output_channels_ = 0;

        // ASIO-specific data (using void* to avoid header issues)
        void* driver_info_ptr_ = nullptr;
        void* buffer_infos_ptr_ = nullptr;
        void* callbacks_ptr_ = nullptr;
        void* input_channel_infos_ptr_ = nullptr;
        void* output_channel_infos_ptr_ = nullptr;

        // Audio buffers
        std::vector<std::vector<float>> input_buffers_;
        std::vector<std::vector<float>> output_buffers_;

        // Performance monitoring
        std::chrono::high_resolution_clock::time_point last_callback_time_;
        std::atomic<int> callback_count_{ 0 };

    public:
        ASIOInterface();
        ~ASIOInterface() override;

        // AudioInterface implementation
        bool initialize(int sample_rate = SAMPLE_RATE_96K,
            int buffer_size = BUFFER_SIZE_ULTRA_LOW) override;
        void shutdown() override;
        bool isInitialized() const override;

        HardwareType getType() const override;
        std::string getName() const override;
        std::string getDriverName() const override;
        int getInputChannelCount() const override;
        int getOutputChannelCount() const override;
        std::vector<int> getSupportedSampleRates() const override;
        std::vector<int> getSupportedBufferSizes() const override;

        bool startStreaming(AudioProcessor* processor) override;
        void stopStreaming() override;
        bool isStreaming() const override;

        double getCurrentLatency() const override;
        PerformanceMetrics getMetrics() const override;

        void enableLowLatencyMode() override;
        bool supportsHardwareDSP() const override;
        void configureDSP() override;

        bool setInputChannel(int channel, bool enabled) override;
        bool setOutputChannel(int channel, bool enabled) override;
        bool setChannelGain(int channel, float gain) override;

    private:
        // ASIO-specific methods
        bool initializeASIO();
        bool setupASIOBuffers();
        void cleanupASIO();
        HardwareType detectHardwareType(const std::string& driver_name);

        // Static ASIO callbacks (must be static for C API)
        static void bufferSwitch(long doubleBufferIndex, long directProcess);
        static void sampleRateDidChange(double sRate);
        static long asioMessage(long selector, long value, void* message, double* opt);
        static long bufferSwitchTimeInfo(void* params, long doubleBufferIndex, long directProcess);

        // Instance method for actual processing
        void processAudioCallback(long bufferIndex);

        // Helper methods
        void updatePerformanceMetrics();
        bool loadASIODriver();
        void releaseASIODriver();
    };

    // Helper functions for ASIO integration
    std::vector<std::string> getAvailableASIODrivers();
    bool isASIODriverAvailable(const std::string& driver_name);
    std::unique_ptr<ASIOInterface> createASIOInterface();

} // namespace Syntri