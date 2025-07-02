// Fixed src/core/audio_interface.cpp - matches your project structure
#include "syntri/types.h"
#include "syntri/audio_interface.h"
#include <iostream>
#include <algorithm>  // Add this missing include
#include <string>
#include <memory>

#ifdef ENABLE_ASIO_SUPPORT
#include "syntri/asio_interface.h"
#endif

namespace Syntri {

    std::vector<HardwareType> detectAvailableHardware() {
        std::vector<HardwareType> available_hardware;

        std::cout << "Scanning for professional audio hardware..." << std::endl;

#ifdef ENABLE_ASIO_SUPPORT
        // Try to detect real ASIO hardware
        try {
            auto asio_interface = std::make_unique<ASIOInterface>();
            if (asio_interface && asio_interface->initialize()) {
                auto detected_type = asio_interface->getType();
                available_hardware.push_back(detected_type);

                std::cout << "   Found: " << hardwareTypeToString(detected_type) << std::endl;
                asio_interface->shutdown();
            }
        }
        catch (const std::exception& e) {
            std::cout << "   ASIO detection failed: " << e.what() << std::endl;
        }
#endif

        // Always add stub interface as fallback
        if (available_hardware.empty()) {
            available_hardware.push_back(HardwareType::GENERIC_ASIO);
            std::cout << "   No hardware detected, using simulation mode" << std::endl;
        }

        return available_hardware;
    }

    std::unique_ptr<AudioInterface> createAudioInterface(HardwareType type) {
        std::cout << "Creating audio interface for: " << hardwareTypeToString(type) << std::endl;

#ifdef ENABLE_ASIO_SUPPORT
        // Try real ASIO interface first
        if (type != HardwareType::GENERIC_ASIO) {
            try {
                auto asio_interface = std::make_unique<ASIOInterface>();
                if (asio_interface && asio_interface->initialize()) {
                    std::cout << "   Real ASIO interface created successfully" << std::endl;
                    // Fix: Use move() to convert unique_ptr<ASIOInterface> to unique_ptr<AudioInterface>
                    return std::unique_ptr<AudioInterface>(asio_interface.release());
                }
            }
            catch (const std::exception& e) {
                std::cout << "   ASIO interface creation failed: " << e.what() << std::endl;
            }
        }
#endif

        // Fallback to stub interface
        std::cout << "   Using stub interface (simulation mode)" << std::endl;
        return createStubInterface();
    }

    

    // Stub interface implementation for fallback
    class StubAudioInterface : public AudioInterface {
    private:
        bool initialized_ = false;
        bool streaming_ = false;
        HardwareType detected_type_ = HardwareType::GENERIC_ASIO;
        SimpleMetrics metrics_;

    public:
        ~StubAudioInterface() override = default;

        bool initialize(int sample_rate, int buffer_size) override {
            std::cout << "   Stub interface initialized (simulation mode)" << std::endl;
            std::cout << "      Sample Rate: " << sample_rate << " Hz" << std::endl;
            std::cout << "      Buffer Size: " << buffer_size << " samples" << std::endl;
            initialized_ = true;
            return true;
        }

        void shutdown() override {
            if (streaming_) stopStreaming();
            initialized_ = false;
            std::cout << "   Stub interface shutdown" << std::endl;
        }

        bool isInitialized() const override { return initialized_; }

        HardwareType getType() const override { return detected_type_; }
        std::string getName() const override { return "Stub Audio Interface"; }
        int getInputChannelCount() const override { return 8; }
        int getOutputChannelCount() const override { return 8; }

        bool startStreaming(AudioProcessor* processor) override {
            if (!initialized_ || !processor) return false;

            streaming_ = true;
            std::cout << "   Stub streaming started (no real audio)" << std::endl;
            return true;
        }

        void stopStreaming() override {
            streaming_ = false;
            std::cout << "   Stub streaming stopped" << std::endl;
        }

        bool isStreaming() const override { return streaming_; }

        double getCurrentLatency() const override {
            return 2.5; // Simulated latency for testing
        }

        SimpleMetrics getMetrics() const override { return metrics_; }
    };

    std::unique_ptr<AudioInterface> createStubInterface() {
        return std::unique_ptr<AudioInterface>(new StubAudioInterface());
    }

} // namespace Syntri