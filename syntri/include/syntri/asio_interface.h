// include/syntri/asio_interface.h
// Perfect ASIO Integration - No Redefinitions, Clean Dependencies
// Compatible with ASIO SDK 2.3 and your working foundation
#pragma once

#include "syntri/types.h"  // Your working foundation - no conflicts
#include <memory>
#include <vector>
#include <string>

// Forward declarations only - no redefinitions
namespace Syntri {
    class AudioProcessor;
    class AudioInterface;
}

// ASIO-specific namespace to avoid conflicts
namespace Syntri::ASIO {

    // ASIO-specific forward declarations (no ASIO includes in header)
    struct ASIODriverInfo;
    struct ASIOBufferInfo;
    struct ASIOCallbacks;
    struct ASIOChannelInfo;
    typedef long ASIOError;
    typedef double ASIOSampleRate;

    // ASIO Interface Implementation - inherits from your AudioInterface
    class ASIOInterface : public AudioInterface {
    private:
        // Internal state management
        bool initialized_;
        bool streaming_;
        bool asio_available_;
        int sample_rate_;
        int buffer_size_;
        AudioProcessor* processor_;

        // ASIO-specific data (opaque pointers to avoid header dependencies)
        struct ASIOData;
        std::unique_ptr<ASIOData> asio_data_;

        // Internal methods
        bool loadASIOSDK();
        bool initializeASIODriver();
        void setupASIOCallbacks();
        bool createASIOBuffers();
        void cleanupASIO();

        // Static ASIO callbacks (required by ASIO SDK)
        static void bufferSwitchCallback(long doubleBufferIndex, long directProcess);
        static void sampleRateDidChangeCallback(ASIOSampleRate sRate);
        static long asioMessageCallback(long selector, long value, void* message, double* opt);
        static ASIODriverInfo* bufferSwitchTimeInfoCallback(ASIODriverInfo* params, long doubleBufferIndex, long directProcess);

        // Instance management for callbacks
        static ASIOInterface* current_instance_;

    public:
        ASIOInterface();
        virtual ~ASIOInterface();

        // AudioInterface implementation - matches your working interface exactly
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
        bool isASIOAvailable() const { return asio_available_; }
        std::vector<std::string> getAvailableASIODrivers() const;
        bool selectASIODriver(const std::string& driver_name);
        void showASIOControlPanel();

        // Hardware detection helpers
        static bool detectASIOHardware();
        static std::vector<HardwareType> getDetectedHardware();
    };

    // Factory functions for ASIO integration
    std::unique_ptr<AudioInterface> createASIOInterface();
    std::unique_ptr<AudioInterface> createASIOInterfaceForDriver(const std::string& driver_name);

    // ASIO system utilities
    bool initializeASIOSystem();
    void shutdownASIOSystem();
    std::vector<std::string> enumerateASIODrivers();

    // Hardware-specific ASIO interface creators
    std::unique_ptr<AudioInterface> createApolloInterface();     // UAD Apollo series
    std::unique_ptr<AudioInterface> createAvantisInterface();    // Allen & Heath Avantis
    std::unique_ptr<AudioInterface> createX32Interface();        // Behringer X32
    std::unique_ptr<AudioInterface> createScarlettInterface();   // Focusrite Scarlett
    std::unique_ptr<AudioInterface> createBabyfaceInterface();   // RME Babyface Pro

} // namespace Syntri::ASIO