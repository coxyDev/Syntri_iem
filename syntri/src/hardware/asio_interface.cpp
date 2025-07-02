// src/core/audio_interface.cpp
// Clean update that adds ASIO integration to your working foundation
#include "syntri/types.h"
#include "syntri/audio_interface.h"
#include <iostream>
#include <string>
#include <memory>

// Only include ASIO interface if ASIO support is enabled
#ifdef ENABLE_ASIO_SUPPORT
#include "syntri/asio_interface.h"
#endif

namespace Syntri {

    std::vector<HardwareType> detectAvailableHardware() {
        std::vector<HardwareType> available_hardware;

        std::cout << "Detecting available audio hardware..." << std::endl;

#ifdef ENABLE_ASIO_SUPPORT
        // Try to detect real ASIO hardware first
        try {
            auto asio_interface = createASIOInterface();
            if (asio_interface && asio_interface->initialize()) {
                auto detected_type = asio_interface->getType();
                available_hardware.push_back(detected_type);

                std::cout << "Found ASIO hardware: " << hardwareTypeToString(detected_type) << std::endl;
                asio_interface->shutdown();
            }
            else {
                std::cout << "ASIO hardware detection failed" << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cout << "ASIO detection error: " << e.what() << std::endl;
        }
#endif

        // Always include generic ASIO as fallback
        if (available_hardware.empty()) {
            available_hardware.push_back(HardwareType::GENERIC_ASIO);
            std::cout << "Using generic audio interface" << std::endl;
        }

        return available_hardware;
    }

    std::unique_ptr<AudioInterface> createAudioInterface(HardwareType type) {
        std::cout << "Creating audio interface for: " << hardwareTypeToString(type) << std::endl;

#ifdef ENABLE_ASIO_SUPPORT
        // Try real ASIO interface for non-generic types
        if (type != HardwareType::GENERIC_ASIO) {
            try {
                auto asio_interface = createASIOInterface();
                if (asio_interface && asio_interface->initialize()) {
                    std::cout << "Real ASIO interface created successfully" << std::endl;
                    return std::unique_ptr<AudioInterface>(asio_interface.release());
                }
                else {
                    std::cout << "ASIO interface creation failed, falling back to generic" << std::endl;
                }
            }
            catch (const std::exception& e) {
                std::cout << "ASIO interface error: " << e.what() << " - falling back to generic" << std::endl;
            }
        }
#endif

        // Create generic interface (your original working stub)
        std::cout << "Creating generic interface" << std::endl;
        return createStubInterface();
    }

    // Stub interface implementation (your original working code)
    class StubAudioInterface : public AudioInterface {
    private:
        bool initialized_ = false;
        bool streaming_ = false;
        HardwareType detected_type_ = HardwareType::GENERIC_ASIO;
        SimpleMetrics metrics_;

    public:
        ~StubAudioInterface() override = default;

        bool initialize(int sample_rate, int buffer_size) override {
            std::cout << "Initializing generic interface (simulation mode)" << std::endl;
            std::cout << "   Sample Rate: " << sample_rate << " Hz" << std::endl;
            std::cout << "   Buffer Size: " << buffer_size << " samples" << std::endl;
            initialized_ = true;
            return true;
        }

        void shutdown() override {
            if (streaming_) stopStreaming();
            initialized_ = false;
            std::cout << "Generic interface shutdown" << std::endl;
        }

        bool isInitialized() const override { return initialized_; }

        HardwareType getType() const override { return detected_type_; }
        std::string getName() const override { return "Generic Audio Interface"; }
        int getInputChannelCount() const override { return 8; }
        int getOutputChannelCount() const override { return 8; }
        double getCurrentLatency() const override { return 2.5; }

        bool startStreaming(AudioProcessor* processor) override {
            if (!initialized_ || !processor) return false;
            streaming_ = true;
            std::cout << "Generic streaming started (simulation)" << std::endl;
            return true;
        }

        void stopStreaming() override {
            streaming_ = false;
            std::cout << "Generic streaming stopped" << std::endl;
        }

        bool isStreaming() const override { return streaming_; }
        SimpleMetrics getMetrics() const override { return metrics_; }
    };

    std::unique_ptr<AudioInterface> createStubInterface() {
        return std::unique_ptr<AudioInterface>(new StubAudioInterface());
    }

} // namespace Syntri