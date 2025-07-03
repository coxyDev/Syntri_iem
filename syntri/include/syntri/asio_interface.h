// include/syntri/asio_interface.h
// ASIO interface that matches your existing type structure
#pragma once

#include "syntri/audio_interface.h"
#include <vector>
#include <memory>
#include <chrono>

// Forward declarations to avoid header conflicts
#ifdef ENABLE_ASIO_SUPPORT
class AsioDrivers;
#endif

namespace Syntri {

    /**
     * @brief ASIO-based audio interface for professional hardware integration
     *
     * This class provides real-time audio streaming using the ASIO (Audio Stream
     * Input/Output) protocol, enabling low-latency communication with professional
     * audio hardware like UAD Apollo, Allen & Heath Avantis, Behringer X32, etc.
     *
     * Uses your existing type structure:
     * - HardwareType enum (not HardwareInfo struct)
     * - MultiChannelBuffer for audio data
     * - SimpleMetrics for performance tracking
     */
    class ASIOInterface : public AudioInterface {
    public:
        ASIOInterface();
        virtual ~ASIOInterface();

        // AudioInterface implementation - matches your existing interface
        bool initialize(int sample_rate = SAMPLE_RATE_96K, int buffer_size = BUFFER_SIZE_ULTRA_LOW) override;
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

        // ASIO-specific methods
        std::vector<std::string> getAvailableDrivers() const;
        bool loadDriver(const std::string& driver_name);
        void unloadDriver();
        std::string getCurrentDriverName() const { return current_driver_name_; }

        // Hardware detection helper - returns HardwareType, not HardwareInfo
        std::vector<HardwareType> detectHardwareTypes() const;

    private:
        // ASIO SDK integration
#ifdef ENABLE_ASIO_SUPPORT
        std::unique_ptr<AsioDrivers> asio_drivers_;
#endif

        // ASIO state
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

        // Processor callback
        AudioProcessor* processor_;

        // Performance monitoring
        mutable std::chrono::high_resolution_clock::time_point last_callback_time_;
        mutable long callback_count_;

        // Audio buffers - using your MultiChannelBuffer type
        MultiChannelBuffer input_buffers_;
        MultiChannelBuffer output_buffers_;

        // Hardware detection
        HardwareType detected_type_;

        // Internal methods
        bool initializeASIO();
        void cleanupASIO();
        bool configureBuffers();
        void releaseBuffers();

        // ASIO callbacks (static functions for C interface)
        static void bufferSwitch(long index, long processNow);
        static void sampleRateChanged(double sRate);
        static long asioMessage(long selector, long value, void* message, double* opt);
        static long bufferSwitchTimeInfo(void* params, long index, long processNow);

        // Internal callback handling
        void handleBufferSwitch(long buffer_index);
        void processAudioCallback(int frame_count);

        // Helper methods
        HardwareType detectHardwareType(const std::string& driver_name) const;
        std::string getDriverDescription(const std::string& driver_name) const;
    };

} // namespace Syntri