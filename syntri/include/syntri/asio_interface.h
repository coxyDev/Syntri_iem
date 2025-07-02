// include/syntri/asio_interface.h
// Clean ASIO integration that builds on your working foundation
#pragma once

#include "syntri/types.h"
#include "syntri/audio_interface.h"
#include <memory>
#include <vector>
#include <atomic>
#include <chrono>
#include <string>

namespace Syntri {

    // ASIO Interface - Real hardware communication when ASIO SDK is available
    class ASIOInterface : public AudioInterface {
    private:
        // Core state
        bool initialized_ = false;
        bool streaming_ = false;
        HardwareType detected_type_ = HardwareType::GENERIC_ASIO;
        SimpleMetrics metrics_;
        AudioProcessor* processor_ = nullptr;

        // Audio configuration
        int sample_rate_ = SAMPLE_RATE_96K;
        int buffer_size_ = BUFFER_SIZE_ULTRA_LOW;
        int input_channels_ = 0;
        int output_channels_ = 0;

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
        int getInputChannelCount() const override;
        int getOutputChannelCount() const override;
        double getCurrentLatency() const override;

        bool startStreaming(AudioProcessor* processor) override;
        void stopStreaming() override;
        bool isStreaming() const override;

        SimpleMetrics getMetrics() const override;

    private:
        // ASIO-specific methods
        bool initializeASIO();
        void cleanupASIO();
        HardwareType detectHardwareType(const std::string& driver_name);

        // Static ASIO callbacks
        static void bufferSwitch(long doubleBufferIndex, long directProcess);
        static void sampleRateDidChange(double sRate);
        static long asioMessage(long selector, long value, void* message, double* opt);

        // Instance method for actual processing
        void processAudioCallback(long bufferIndex);
    };

    // Factory function for ASIO interface
    std::unique_ptr<ASIOInterface> createASIOInterface();

} // namespace Syntri