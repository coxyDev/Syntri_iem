// include/syntri/audio_interface.h
// Fixed version with clean type definitions and proper inheritance
#pragma once

#include "syntri/types.h"
#include <vector>
#include <string>
#include <memory>

namespace Syntri {

    // Simple audio processing callback interface
    class AudioProcessor {
    public:
        virtual ~AudioProcessor() = default;

        // Main audio processing callback - must be real-time safe
        virtual void processAudio(
            const MultiChannelBuffer& inputs,
            MultiChannelBuffer& outputs,
            int num_samples
        ) = 0;

        // Called when audio parameters change
        virtual void setupChanged(int sample_rate, int buffer_size) = 0;
    };

    // Hardware abstraction interface - clean and simple
    class AudioInterface {
    public:
        virtual ~AudioInterface() = default;

        // Basic lifecycle
        virtual bool initialize(int sample_rate = SAMPLE_RATE_96K, int buffer_size = BUFFER_SIZE_ULTRA_LOW) = 0;
        virtual void shutdown() = 0;
        virtual bool isInitialized() const = 0;

        // Hardware information
        virtual HardwareType getType() const = 0;
        virtual std::string getName() const = 0;
        virtual int getInputChannelCount() const = 0;
        virtual int getOutputChannelCount() const = 0;
        virtual double getCurrentLatency() const = 0;

        // Audio streaming
        virtual bool startStreaming(AudioProcessor* processor) = 0;
        virtual void stopStreaming() = 0;
        virtual bool isStreaming() const = 0;

        // Performance monitoring
        virtual SimpleMetrics getMetrics() const = 0;
    };

    // Factory functions for creating hardware interfaces
    std::unique_ptr<AudioInterface> createAudioInterface(HardwareType type);
    std::unique_ptr<AudioInterface> createStubInterface();

    // Audio processor factory
    std::unique_ptr<AudioProcessor> createTestProcessor(bool generate_tone = false);

    // Hardware detection - returns HardwareType enum
    std::vector<HardwareType> detectAvailableHardware();

    // Utility functions
    void printHardwareInfo(HardwareType type);
    bool runBasicHardwareTest();

} // namespace Syntri